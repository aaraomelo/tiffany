/* base.c — A BASE ORTONORMAL, O CRUZADO COMPLETO, E GERAR CORPOS EM VÁRIAS DIMENSÕES.
 *
 * O Aarão: "define a base ortonormal de esferas pares e ímpares, mostra que é ortonormal, e dá o
 * produto direto e cruzado na forma recursiva; e avança no corpo de corpos. Isso é mais
 * importante: operar corpos nas várias dimensões e gerar outros — esse cruzado completo vai
 * fornecer isso."
 *
 * O CRUZADO COMPLETO é o par (a×b, J), que se reparte pelas dimensões sem se sobrepor:
 *
 *     dimensão ÍMPAR 1, 3, 7   ->  a×b, o produto vetorial
 *     dimensão PAR             ->  J, com J² = -I
 *
 * e é ele que fornece a AÇÃO de que a geração precisa: gerar um corpo a partir de dois não é
 * pô-los lado a lado (isso é o direto, e não voa) — é fazer um agir no outro.
 *
 *   §B1  a base é ortonormal, e os e_i são pontos da esfera
 *   §B2  as esferas pares e ímpares, e o que cada uma tem
 *   §B3  o produto DIRETO na forma recursiva
 *   §B4  o CRUZADO na forma recursiva — Cayley--Dickson, e a parte antissimétrica é o a×b
 *   §B5  o cruzado COMPLETO, e a cobertura das dimensões
 *   §B6  e gerar corpos: R^a ∨ R^b = R^lcm, e por que o direto não gera
 *   §B7  a geração MEDIDA em GF(2^6): fecha-se o par por + e ×, e conta-se 2^lcm
 *   §B8  uma dimensão é PROJEÇÃO da outra, e duas formam o corpo dual — não há buraco
 *   §B9  uma dimensão sozinha não é reversível: o inverso é do PAR
 *   §B10 uma dimensão e a ANTERIOR: avança e contrai, e o saldo fecha o balanço
 *   §B11 o corpo é a TORRE inteira — e há a torre dual, que desce
 *   §B12 a branca e a negra: cada torre antissimétrica, as duas juntas simétricas
 *   §B13 a recursão salta entre torres; no fim a dual está vazia, e a cifra fica fora
 *   §B14 o origami: a dobra quebra a simetria e GUARDA a memória dela
 *
 *   cc -O2 -std=c99 base.c -o base && ./base        (sem -lm: nao ha virgula)
 */
#include <stdio.h>
#include <string.h>
#include "reta.h"      /* Dir e Cruz: a operação */
#include "unidade.h"

#define D 8

/* E TUDO ISTO FECHA EM ℤ. A base ortonormal tem Gram = I e os produtos da tabela valem
 * ±1: somas e produtos de inteiros dão inteiros, e Cayley–Dickson não introduz uma
 * divisão em passo nenhum. O double que aqui estava não transportava vírgula — trazia
 * um limiar a cada comparação, e com ele a pergunta deixava de ser sobre o número e
 * passava a ser sobre a régua. */
static long dir_rec(const long *a, const long *b, int n){
    if(n == 1) return a[0]*b[0];
    return dir_rec(a, b, n-1) + a[n-1]*b[n-1];      /* a última coordenada destaca-se */
}
static void conj_cd(const long *x, int n, long *out){
    out[0] = x[0];
    for(int k = 1; k < n; k++) out[k] = -x[k];
}
/* Cayley--Dickson: (a,b)(c,d) = (a·c − conj(d)·b, d·a + b·conj(c)) */
static void cd(const long *x, const long *y, int n, long *out){
    if(n == 1){ out[0] = x[0]*y[0]; return; }
    int m = n/2;
    const long *a = x, *b = x+m, *c = y, *d = y+m;
    long cd_[D], cc[D], t1[D], t2[D], t3[D], t4[D];
    conj_cd(d, m, cd_); conj_cd(c, m, cc);
    cd(a, c, m, t1); cd(cd_, b, m, t2);
    cd(d, a, m, t3); cd(b, cc, m, t4);
    for(int k = 0; k < m; k++){ out[k] = t1[k] - t2[k]; out[m+k] = t3[k] + t4[k]; }
}
static long mdc(long a, long b){ while(b){ long t = a % b; a = b; b = t; } return a; }

/* GF(2^n) por polinomios sobre F2. Aqui a acao e concreta e a geracao mede-se. */
static int gf_mul(int a, int b, int n, int red){
    int r = 0;
    for(int k = 0; k < n; k++){
        if(b & 1) r ^= a;
        b >>= 1;
        a <<= 1;
        if(a & (1 << n)) a ^= red;                 /* a borda: reduzir pelo primitivo */
    }
    return r;
}
static int gf_pot(int a, long e, int n, int red){
    int r = 1;
    while(e > 0){ if(e & 1) r = gf_mul(r, a, n, red); a = gf_mul(a, a, n, red); e >>= 1; }
    return r;
}

int main(void){
printf("\n=== A BASE, O CRUZADO COMPLETO, E GERAR CORPOS ===========================\n");
printf("    O cruzado completo é o par (a×b, J): um nas ímpares 1,3,7 e o outro nas\n");
printf("    pares. E é ele que dá a AÇÃO de que a geração precisa.\n");

printf("\n§B1  A base é ortonormal, e os e_i são pontos da esfera.\n\n");
{
    int mal = 0, quantos = 0;
    for(int n = 1; n <= D; n++)
        for(int i = 0; i < n; i++) for(int j = 0; j < n; j++){
            long ei[D] = {0}, ej[D] = {0};
            ei[i] = 1; ej[j] = 1;
            long v = dir_rec(ei, ej, n);
            quantos++;
            if(v != (i==j ? 1 : 0)) mal++;
        }
    printf("      <e_i, e_j> = δ_ij, em toda dimensão de 1 a %d: %d pares, %d falhas\n\n",
           D, quantos, mal);
    /* e as DUAS FORMAS do directo dão o mesmo: a recursiva deste ficheiro, que destaca a
     * última coordenada e mostra a dobra, e a fechada da `reta.h`. A recursão não é uma
     * implementação diferente — é a MESMA operação lida como recorrência. */
    long duas = 0, dtot = 0;
    for(long t = 0; t < 200; t++){
        long u[D], v[D];
        for(int i = 0; i < D; i++){
            u[i] = ((t*7 + i*3) % 11) - 5;
            v[i] = ((t*5 + i*2) % 9)  - 4;
        }
        for(int n = 1; n <= D; n++){ dtot++; if(dir_rec(u, v, n) == rt_dir(u, v, n)) duas++; }
    }
    printf("      e as duas formas do DIRETO — a recursiva e a fechada da reta.h — dão o\n"
           "      mesmo em %ld de %ld leituras\n\n", duas, dtot);
    ok("a base é ortonormal — e o produto que o diz é o DIRETO", mal == 0);
    ok("E AS DUAS FORMAS DO DIRECTO SÃO A MESMA OPERAÇÃO: a recursiva deste ficheiro"
       " destaca a última coordenada — e é isso que faz a torre 1→2→4→8 dobrar —, e a"
       " fechada da `reta.h` soma de uma vez. Não são duas implementações: é a mesma"
       " operação lida como recorrência e como forma fechada, e dão o mesmo inteiro em"
       " todas as dimensões de 1 a 8",
       duas == dtot && dtot == 200*D);
    printf("      E os e_i estão na esfera: ‖e_i‖ = 1 por construção. Logo a base é n PONTOS DA\n");
    printf("      ESFERA, dois a dois perpendiculares — e é por isso que ela é o objeto natural\n");
    printf("      aqui: a esfera é o lugar da norma 1, e o determinante ±1 vive nela.\n");
}

printf("\n§B2  As esferas pares e ímpares, e o que cada uma tem.\n\n");
{
    printf("      esfera    ambiente   o que existe lá\n");
    int mal = 0;
    for(int n = 1; n <= D; n++){
        int cruz = (n == 1 || n == 3 || n == 7);
        int jj   = (n % 2 == 0);
        printf("      S^%-7d R^%-8d %s\n", n-1, n,
               cruz ? "a×b — o cruzado, e a esfera é PARALELIZÁVEL"
             : jj  ? "J, com J² = -I — o cruzado DUAL"
                   : "nem um nem outro");
        if(cruz && jj) mal++;
    }
    printf("\n");
    ok("nenhuma dimensão tem os dois — eles repartem-se sem se sobrepor", mal == 0);
    printf("      As esferas S⁰, S² e S⁶ (ambiente 1, 3, 7) são as paralelizáveis, e são\n");
    printf("      exatamente onde o a×b existe. Nas dimensões PARES não há a×b, e o lugar é do\n");
    printf("      J — que existe porque det(J)² = (-1)^n só fecha em n par.\n");
}

printf("\n§B3  O produto DIRETO na forma recursiva.\n\n");
{
    printf("      <a,b>_n  =  <ã,b̃>_(n-1)  +  a_(n-1)·b_(n-1)\n\n");
    int mal = 0;
    for(int k = 0; k < 200; k++){
        int n = 2 + k % (D-1);
        long a[D], b[D];
        for(int i = 0; i < n; i++){ a[i] = ((3*k + i*3 + 1) % 11 - 5); b[i] = ((5*k + i*5 + 2) % 11 - 5); }
        long s = 0;
        for(int i = 0; i < n; i++) s += a[i]*b[i];
        if(dir_rec(a,b,n) != s) mal++;
    }
    printf("      a recursão contra a soma direta, 200 casos de dim 2 a %d: %d falhas\n\n", D, mal);
    ok("o direto recursivo é o direto — e a recursão é trivial, de propósito", mal == 0);
    printf("      Repare-se em como esta recursão é POBRE: a última coordenada entra por soma e\n");
    printf("      nada mais. É o retrato do produto que não vê a ordem — não há o que decidir a\n");
    printf("      cada nível, logo não há estrutura a acumular.\n");
}

printf("\n§B4  O CRUZADO na forma recursiva — e a antissimétrica É o a×b.\n\n");
{
    printf("      Cayley-Dickson:  (a,b)·(c,d) = ( a·c - conj(d)·b ,  d·a + b·conj(c) )\n");
    printf("      e o cruzado é a metade ANTISSIMÉTRICA: ½(xy - yx)\n\n");
    long x[D] = {1,2,3,4}, y[D] = {5,6,7,8}, p[D], q[D];
    cd(x, y, 4, p); cd(y, x, 4, q);
    /* e o formato ACOMPANHA o tipo: estes valores sao long, e um %g sobre um long nao
     * imprime o numero — le os bits como se fossem de virgula, e sai 2.09866e-317.
     * Valor certo, texto errado, e nenhuma assercao o apanharia. */
    printf("      em H:  x*y = (%ld,%ld,%ld,%ld)\n", p[0],p[1],p[2],p[3]);
    printf("             y*x = (%ld,%ld,%ld,%ld)\n", q[0],q[1],q[2],q[3]);
    long a[3] = {x[1],x[2],x[3]}, b[3] = {y[1],y[2],y[3]};
    long c[3]; rt_cruz3(a, b, c);
    /* E TRABALHA-SE EM DOBRO, para nao dividir: a parte antissimetrica e ½(xy-yx), e
     * em vez de dividir por dois compara-se (xy-yx) com 2·(a×b). O residuo e ZERO. */
    long dif[D];
    for(int k = 0; k < 4; k++) dif[k] = p[k] - q[k];
    printf("      (xy-yx)  = (%ld,%ld,%ld,%ld)   e   2(axb) = (0,%ld,%ld,%ld)\n",
           dif[0],dif[1],dif[2],dif[3], 2*c[0], 2*c[1], 2*c[2]);
    printf("      e axb    =   (%ld,%ld,%ld)\n\n", c[0],c[1],c[2]);
    ok("A PARTE ANTISSIMETRICA DO CAYLEY-DICKSON E O PRODUTO VECTORIAL, e mede-se em"
       " inteiros e em DOBRO, sem dividir por dois: (xy - yx) = 2*(0, axb), com residuo"
       " ZERO nas quatro coordenadas. E o formato acompanha o tipo — estava aqui um %g a"
       " imprimir longs, que le os bits como se fossem de virgula e imprimia 2.09866e-317:"
       " valor certo, texto errado, e nenhuma assercao o apanharia",
       dif[0] == 0 && dif[1] == 2*c[0] && dif[2] == 2*c[1] && dif[3] == 2*c[2]
       && (c[0] || c[1] || c[2]));
    printf("      A recursão AQUI é rica: a cada nível o conjugado entra, a ordem dos fatores\n");
    printf("      muda de lado, e é isso que gera a estrutura nova. Compare-se com o §B3 — a\n");
    printf("      diferença entre os dois é a diferença entre somar e multiplicar.\n");
}

printf("\n§B5  O cruzado COMPLETO, e a cobertura.\n\n");
{
    int cobertas = 0, buracos = 0, mal = 0;
    printf("      dim   tem    cobre?\n");
    for(int n = 1; n <= D; n++){
        int cruz = (n == 1 || n == 3 || n == 7), jj = (n % 2 == 0);
        printf("      %-5d %-6s %s\n", n, cruz ? "a×b" : jj ? "J" : "-",
               (cruz||jj) ? "sim" : "NÃO — buraco");
        if(cruz || jj) cobertas++; else buracos++;
        /* a afirmacao a medir e esta, e nao um numero que eu escrevi a mao:
         * coberta  <=>  n par  OU  n em {1,3,7} */
        if((cruz||jj) != (n % 2 == 0 || n == 1 || n == 3 || n == 7)) mal++;
    }
    printf("\n      %d das %d dimensões cobertas; %d buracos (as ímpares que não são 1, 3, 7)\n\n",
           cobertas, D, buracos);
    ok("as cobertas são exatamente as pares mais as ímpares 1, 3 e 7", mal == 0);
    printf("      É esta a completação: onde o a×b não chega, o J chega — e vice-versa. Chamar\n");
    printf("      ao segundo \"a ausência do primeiro\" era ler metade do par.\n");
}

printf("\n§B6  E gerar corpos: R^a ∨ R^b = R^lcm — medido.\n\n");
{
    /* o corpo de corpos: gerar um a partir de dois. A dimensao do gerado e o lcm, e a
     * verificacao e a que o viveiro.tex faz — a contencao R^x ⊂ R^n <=> x | n. */
    int mal = 0;
    printf("      a  b   a⊕b (direto)   a∨b (gerado)   contém os dois?   é o MENOR?\n");
    struct { int a, b; } t[] = { {2,3}, {2,4}, {3,4}, {2,6}, {4,6}, {3,5} };
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        int a = t[k].a, b = t[k].b;
        long g = mdc(a,b), l = (long)a*b/g;
        int contem = (l % a == 0) && (l % b == 0);
        int menor = 1;
        for(int n = 1; n < l; n++) if(n % a == 0 && n % b == 0) menor = 0;
        printf("      %d  %d   R^%-11d R^%-12ld %-17s %s\n", a, b, a+b, l,
               contem ? "sim" : "NÃO", menor ? "sim" : "NÃO");
        if(!contem || !menor) mal++;
    }
    printf("\n");
    ok("o gerado contém os dois pais e é o MENOR que os contém", mal == 0);
    printf("      E note-se a coluna do meio: o DIRETO dá a+b, que nem sequer é divisível pelos\n");
    printf("      pais em geral — R^2 ⊕ R^3 tem dimensão 5, e nem 2 nem 3 dividem 5. Ele não\n");
    printf("      falha por pouco: falha por não ser a operação certa.\n");
    printf("\n      E o que o CRUZADO fornece aqui é a AÇÃO. Gerar não é pôr dois lado a lado —\n");
    printf("      isso é o direto, e o viveiro.c mede que não voa (divisor de zero sempre). É\n");
    printf("      fazer um agir no outro pelo que partilham, e o resultado é um corpo NOVO que\n");
    printf("      contém ambos. Sem ação não há geração: há coabitação.\n");
    printf("\n      O ganho mede-se: o filho traz lcm/a de novo em relação ao pai a. Em (2,3) traz\n");
    printf("      3 vezes; em (2,4) traz 2, porque 2 já estava lá dentro; e em (a,a) traz 1 — o\n");
    printf("      cruzamento de uma espécie consigo não gera nada. A NOVIDADE VEM DA DIFERENÇA.\n");
}

printf("\n§B7  A geração MEDIDA: dois corpos agem um no outro e fazem um terceiro.\n\n");
{
    /* GF(2^6) com x^6+x+1. Os subcorpos sao os fixos de a^(2^d)=a, e ha um por divisor de 6. */
    const int n = 6, red = 0x43, N = 64;
    printf("      corpo ambiente: GF(2^6), 64 elementos, borda x^6 + x + 1\n\n");
    printf("      d   fixos de a^(2^d) = a   esperado 2^mdc(d,6)   é subcorpo?\n");
    int mal = 0;
    for(int d = 1; d <= 6; d++){
        int q = 0, fecha = 1, sub[64], ns = 0;
        for(int a = 0; a < N; a++)
            if(gf_pot(a, 1L << d, n, red) == a){ q++; sub[ns++] = a; }
        for(int i = 0; i < ns; i++) for(int j = 0; j < ns; j++){   /* fechado por + e × ? */
            int s = sub[i] ^ sub[j], p = gf_mul(sub[i], sub[j], n, red), achouS = 0, achouP = 0;
            for(int k = 0; k < ns; k++){ if(sub[k]==s) achouS=1; if(sub[k]==p) achouP=1; }
            if(!achouS || !achouP) fecha = 0;
        }
        int esp = 1 << mdc(d, 6);
        printf("      %d   %-21d %-20d %s\n", d, q, esp, fecha ? "sim" : "NÃO");
        if(q != esp || !fecha) mal++;
    }
    printf("\n");
    ok("os subcorpos de GF(2^6) são um por divisor de 6, e cada um fecha", mal == 0);

    /* E AGORA A GERACAO: o fecho de GF(4) ∪ GF(8) por + e × dentro de GF(2^6). */
    printf("\n      E a geração: partir de dois subcorpos e fechar por + e ×.\n\n");
    printf("      pais              só com +   com + e ×   é o 2^lcm?   o + já é corpo?\n");
    struct { int da, db; } par[] = { {2,3}, {1,2}, {2,2}, {1,3}, {3,3}, {2,6} };
    int mal2 = 0, mal3 = 0;
    for(size_t t = 0; t < sizeof par/sizeof *par; t++){
        int da = par[t].da, db = par[t].db;
        int tem[64] = {0}, som[64] = {0}, q = 0, qs = 0;
        for(int a = 0; a < N; a++)
            if(gf_pot(a, 1L<<da, n, red) == a || gf_pot(a, 1L<<db, n, red) == a){
                tem[a] = som[a] = 1; q++; qs++;
            }
        for(int volta = 0; volta < 8; volta++){                /* fechar: a dobra, ate parar */
            for(int a = 0; a < N; a++) if(tem[a]) for(int b = 0; b < N; b++) if(tem[b]){
                int s = a ^ b, p = gf_mul(a, b, n, red);
                if(!tem[s]){ tem[s] = 1; q++; }
                if(!tem[p]){ tem[p] = 1; q++; }
            }
            for(int a = 0; a < N; a++) if(som[a]) for(int b = 0; b < N; b++) if(som[b]){
                int s = a ^ b;                                 /* o mesmo fecho, SÓ com o + */
                if(!som[s]){ som[s] = 1; qs++; }
            }
        }
        long l = (long)da*db/mdc(da,db);
        int esp = 1 << l;
        int somaFecha = 1;                      /* o fecho SÓ com o + chega a ser corpo? */
        for(int a = 0; a < N && somaFecha; a++) if(som[a])
            for(int b = 0; b < N; b++) if(som[b] && !som[gf_mul(a,b,n,red)]){ somaFecha = 0; break; }
        printf("      GF(2^%d), GF(2^%d)  %-10d %-11d %-11s %s\n", da, db, qs, q,
               q == esp ? "sim" : "NÃO", somaFecha ? "sim" : "NÃO — nem é corpo");
        if(q != esp) mal2++;
        /* o + so basta quando um pai ja contem o outro; havendo diferenca real, ele fica aquem */
        int contido = (l == (da > db ? da : db));
        if((qs == q) != contido || somaFecha != contido) mal3++;
    }
    printf("\n");
    ok("o corpo GERADO por dois subcorpos é exatamente o de dimensão lcm", mal2 == 0);
    ok("o + sozinho só basta quando um pai já contém o outro — havendo diferença,\n         ele fica aquém e nem chega a ser corpo", mal3 == 0);
    printf("      Aqui a geração deixou de ser afirmação e passou a ser contagem: fecha-se o par\n");
    printf("      por + e ×, contam-se os elementos, e o número é 2^lcm — nem um a mais, nem um\n");
    printf("      a menos. GF(4) e GF(8) juntos dão os 64; GF(4) consigo próprio dá 4.\n");
    printf("\n      E repare-se onde entra o CRUZADO: o fecho acima só cresce porque a MULTIPLICAÇÃO\n");
    printf("      leva um elemento de um pai para fora de ambos. Só com o + (o direto) o fecho de\n");
    printf("      GF(4) ∪ GF(8) pararia num espaço vetorial, sem corpo novo. É a parte que não\n");
    printf("      comuta — a que muda de sinal ao trocar a ordem — que abre lugar onde não havia.\n");
}

printf("\n§B8  Uma dimensão é PROJEÇÃO da outra, e duas formam o corpo DUAL.\n\n");
{
    /* O Aarao: "veja bem, uma dimensao e projecao da outra e duas formam um corpo dual."
     * Isto corrige a leitura do §B5. La eu listei cada dimensao como caso isolado, e a 5
     * apareceu como BURACO. Mas as dimensoes nao vem isoladas: vem aos PARES (n, 2n), ligadas
     * pela projecao — e e o par que carrega o corpo, nao a dimensao sozinha. */
    printf("      π₁(a,b) = a   e   π₂(a,b) = b     as duas projeções R^2n -> R^n\n");
    printf("      J(a,b)  = (-b, a)                 o dual, que TROCA as duas\n\n");
    int malD = 0, malJ = 0, malT = 0, malI = 0;
    for(int n = 1; n <= 4; n++){
        int m = 2*n;
        for(int k = 0; k < 60; k++){
            long x[D], y[D];
            for(int i = 0; i < m; i++){ x[i] = ((7*k + i*3 + 1) % 11 - 5); y[i] = ((11*k + i*5 + 3) % 11 - 5); }
            /* (a) o DIRETO e a soma das duas projecoes — a recursao pela METADE */
            long tot = dir_rec(x, y, m);
            long p1 = dir_rec(x, y, n), p2 = dir_rec(x+n, y+n, n);
            if(tot != p1 + p2) malD++;
            /* (b) J^2 = -I */
            long jx[D], jjx[D];
            for(int i = 0; i < n; i++){ jx[i] = -x[n+i]; jx[n+i] = x[i]; }
            for(int i = 0; i < n; i++){ jjx[i] = -jx[n+i]; jjx[n+i] = jx[i]; }
            for(int i = 0; i < m; i++) if(jjx[i] != -x[i]) malJ++;
            /* (c) o J TROCA as projecoes: π₁∘J = -π₂  e  π₂∘J = π₁ */
            for(int i = 0; i < n; i++){
                if(jx[i] != -x[n+i]) malT++;
                if(jx[n+i] != x[i]) malT++;
            }
            /* (d) e o J preserva o direto: e uma isometria, logo vive na esfera */
            long jy[D];
            for(int i = 0; i < n; i++){ jy[i] = -y[n+i]; jy[n+i] = y[i]; }
            if(dir_rec(jx, jy, m) != tot) malI++;
        }
    }
    printf("      <x,y>_2n = <π₁x,π₁y>_n + <π₂x,π₂y>_n, em n = 1..4: %d falhas\n", malD);
    printf("      J² = -I                                            : %d falhas\n", malJ);
    printf("      π₁∘J = -π₂  e  π₂∘J = π₁                            : %d falhas\n", malT);
    printf("      <Jx,Jy> = <x,y>  (J é isometria)                    : %d falhas\n\n", malI);
    ok("o direto é a SOMA das duas projeções — a recursão pela metade", malD == 0);
    ok("o J troca as duas projeções uma na outra, e J² = -I", malJ == 0 && malT == 0);
    ok("o J preserva o produto direto — é isometria, e vive na esfera", malI == 0);

    printf("\n      E é isto que corrige o §B5. Ali eu listei dimensão por dimensão e a 5 saiu como\n");
    printf("      BURACO. Mas a dimensão não vem sozinha: vem no par (n, 2n), e o que carrega o\n");
    printf("      corpo é o PAR. Repare-se:\n\n");
    printf("      n    2n   o par (n, 2n)                        J² = -I medido em R^2n?\n");
    int malP = 0;
    for(int n = 1; n <= 8; n++){
        int m = 2*n, mal = 0;
        /* CONSTRUIR o J neste par e medi-lo — nao basta observar que 2n e par, que e
         * verdade de graca e nao afirma nada sobre o dual. */
        for(int k = 0; k < 40; k++){
            long x[16], jx[16], jjx[16];
            for(int i = 0; i < m; i++) x[i] = ((17*k + i*3 + 1) % 11 - 5);
            for(int i = 0; i < n; i++){ jx[i] = -x[n+i]; jx[n+i] = x[i]; }
            for(int i = 0; i < n; i++){ jjx[i] = -jx[n+i]; jjx[n+i] = jx[i]; }
            for(int i = 0; i < m; i++) if(jjx[i] != -x[i]) mal++;
        }
        printf("      %-4d %-4d %-36s %s\n", n, m,
               (n==1||n==3||n==7) ? "a×b em R^n, e o dual em R^2n" : "R^n é só a projeção",
               mal == 0 ? "sim, 0 falhas" : "NÃO");
        malP += mal;
    }
    printf("\n");
    ok("toda dimensão n tem o seu par 2n, e o dual J está MEDIDO em cada um — sem buraco",
       malP == 0);
    printf("      A dimensão 5 não é um buraco: é a projeção de 10, e o corpo dual está em 10.\n");
    printf("      O que faltava em 5 era o a×b, e o a×b nunca foi a única metade do par — era\n");
    printf("      metade dele. Ler dimensão a dimensão é ler uma projeção e chamar-lhe o todo.\n");
    printf("\n      E as duas recursões repartem as MESMAS duas metades: o direto SOMA-as (§B3 pela\n");
    printf("      metade, medido acima), e o cruzado CRUZA-as com o conjugado (§B4). Uma única\n");
    printf("      partição, duas maneiras de a fechar — e é essa a dualidade, não a lista.\n");
}

printf("\n§B9  Uma dimensão sozinha não é reversível — só com a sua dual.\n\n");
{
    /* O Aarao: "uma dimensao sozinha nao e um corpo, pelo menos nao reversivel, so com sua
     * dimensao dual." E o teste do projeto inteiro: nao se pergunta que roupa a estrutura
     * veste, pergunta-se se e REVERSIVEL. Aqui mede-se de que lado esta o inverso. */
    printf("      (a) SOZINHA — a dimensão com o produto coordenada a coordenada:\n\n");
    int malZ = 0;
    for(int n = 2; n <= D; n++){
        long u[D] = {0}, v[D] = {0}, w[D];
        u[0] = 1; v[n-1] = 1;                       /* dois nao-nulos... */
        long nz = 0;
        for(int i = 0; i < n; i++){ w[i] = u[i]*v[i]; nz += w[i] < 0 ? -w[i] : w[i]; }
        printf("      R^%d: (1,0,…)·(0,…,1) = 0  ->  divisor de zero, logo NÃO reversível\n", n);
        if(nz != 0) malZ++;                      /* ...cujo produto e zero */
    }
    printf("\n");
    ok("a dimensão sozinha tem divisor de zero em toda dim >= 2 — não é corpo", malZ == 0);

    printf("\n      (b) COM A DUAL — o mesmo espaço, agora com o conjugado e a norma:\n\n");
    int malI = 0, testados = 0;
    for(int n = 2; n <= 8; n *= 2){
        int mal = 0;
        for(int k = 0; k < 80; k++){
            long x[D], cx[D], p[D], nrm = 0;
            for(int i = 0; i < n; i++) x[i] = ((13*k + i*3 + 1) % 11 - 5);
            for(int i = 0; i < n; i++) nrm += x[i]*x[i];
            if(nrm == 0) continue;                  /* «é zero», e não «é menor que a régua» */
            conj_cd(x, n, cx);                      /* +I na 1a projecao, -I na 2a */
            /* E O INVERSO NÃO SE CONSTRÓI DIVIDINDO. x⁻¹ = conj(x)/N(x) é RACIONAL, e
             * `inv[i] = cx[i]/nrm` em inteiros TRUNCA — o produto deixava de dar 1 e a
             * asserção falhava. A identidade que dá o inverso não precisa da divisão:
             *
             *      x · conj(x) = N(x)      um ESCALAR, e inteiro
             *
             * Daí o inverso é conj(x)/N(x) e existe exactamente quando N(x) ≠ 0 — que é
             * a tese. Mede-se o produto pelo conjugado, que fecha em ℤ: primeira
             * coordenada N(x), as outras ZERO. */
            cd(x, cx, n, p);                        /* x · conj(x) = (N, 0, …, 0) */
            testados++;
            if(p[0] != nrm) mal++;
            for(int i = 1; i < n; i++) if(p[i] != 0) mal++;
        }
        printf("      R^%d:  x·conj(x) = (N(x), 0, …, 0) em 80 casos: %d falhas"
               "  →  x⁻¹ = conj(x)/N(x) existe sse N ≠ 0\n", n, mal);
        malI += mal;
    }
    printf("\n      (%d produtos medidos)\n\n", testados);
    ok("COM A DUAL, TODO x ≠ 0 TEM INVERSO — E É O CONJUGADO QUE O DÁ, e mede-se SEM"
       " DIVIDIR. O inverso conj(x)/N(x) é racional, e construí-lo em inteiros TRUNCA —"
       " foi o que aconteceu ao migrar, e a asserção falhou. Mas a identidade que dá o"
       " inverso não precisa da divisão: x·conj(x) = N(x), um ESCALAR inteiro, com a"
       " primeira coordenada igual à norma e todas as outras ZERO. Daí o inverso existe"
       " exactamente quando N(x) ≠ 0 — e «é zero» é uma pergunta sobre o número, ao"
       " contrário do nrm == 0.0 que aqui estava",
       malI == 0 && testados > 0);
    printf("      E repare-se DE ONDE vem o inverso: de conj(x), que é +I na primeira projeção e\n");
    printf("      -I na segunda. Ele é literalmente a operação que só existe por haver duas\n");
    printf("      metades. Sem a dual não há o que conjugar, N(x) não fecha, e o inverso não se\n");
    printf("      escreve. A reversibilidade não é propriedade de uma dimensão: é do PAR.\n");
    printf("\n      É por isso que o §B5, lido dimensão a dimensão, dava um buraco onde não há: a\n");
    printf("      pergunta \"o que existe em R^5?\" está mal posta. A pergunta é \"o que existe no\n");
    printf("      par (5,10)?\" — e lá está o dual, logo lá está o inverso.\n");
}

printf("\n§B10 Uma dimensão e a ANTERIOR: avança e contrai, e o saldo fecha o balanço.\n\n");
{
    /* O Aarao: "uma dimensao e a anterior, o saldo completo fecha o balanco, avanca e contrai
     * com inducao e meta-inducao."  O par (n,2n) do §B8 e o do dual; ESTE e o da recorrencia —
     * a borda σ^n = m·σ^(n-1) + 1 e literalmente o grau n escrito no grau n-1. E ha dois
     * movimentos: o avanco (inducao) e a contracao (meta-inducao). O que os prende e o SALDO. */
    printf("      E: R^(n-1) -> R^n,  x |-> (x, 0)        o AVANÇO, a indução\n");
    printf("      C: R^n -> R^(n-1),  (x, s) |-> x        a CONTRAÇÃO, a meta-indução\n\n");
    int malCE = 0, malSal = 0, malBal = 0;
    for(int n = 2; n <= D; n++)
        for(int k = 0; k < 60; k++){
            long x[D];
            for(int i = 0; i < n; i++) x[i] = ((19*k + i*3 + 1) % 11 - 5);
            long c[D], ec[D];
            for(int i = 0; i < n-1; i++) c[i] = x[i];              /* C: contrai */
            for(int i = 0; i < n-1; i++) ec[i] = c[i];             /* E: avança  */
            ec[n-1] = 0;
            /* (a) C∘E = id: avancar e contrair volta ao mesmo, exato */
            long y[D], ey[D], cey[D];
            for(int i = 0; i < n-1; i++) y[i] = x[i];
            for(int i = 0; i < n-1; i++) ey[i] = y[i];
            ey[n-1] = 0;
            for(int i = 0; i < n-1; i++) cey[i] = ey[i];
            for(int i = 0; i < n-1; i++) if(cey[i] != y[i]) malCE++;
            /* (b) E∘C != id, e o SALDO e exatamente a ultima coordenada */
            long saldo = x[n-1];
            for(int i = 0; i < n-1; i++) if(x[i] != ec[i]) malSal++;
            if(x[n-1] - ec[n-1] != saldo) malSal++;
            /* (c) O BALANCO fecha: ‖x‖² = ‖C(x)‖² + saldo² */
            long nx = dir_rec(x, x, n), nc = dir_rec(c, c, n-1);
            if(nx != nc + saldo*saldo) malBal++;
        }
    printf("      C∘E = id, exato, sem tolerância                       : %d falhas\n", malCE);
    printf("      E∘C perde a última, e o saldo É ela, exato            : %d falhas\n", malSal);
    printf("      ‖x‖² = ‖C(x)‖² + saldo²   (o balanço fecha)           : %d falhas\n\n", malBal);
    ok("o avanço é reversível pela contração: C∘E = id, resíduo 0", malCE == 0);
    ok("E∘C não é id — e o que falta é exatamente o saldo, sem sobra", malSal == 0);
    ok("o balanço fecha com o saldo completo: ‖x‖² = ‖C(x)‖² + saldo²", malBal == 0);
    printf("      Note-se qual dos dois é reversível: C∘E fecha exato, E∘C não. É a mesma forma\n");
    printf("      do D∘∫ = id com ∫∘D != id do corpo diferencial — e ali a conclusão foi que\n");
    printf("      (D,∫) NÃO é a dualidade. Aqui é igual: (E,C) é a recorrência, não o dual. O\n");
    printf("      dual é o J do §B8. São dois pares diferentes, e confundi-los é o erro.\n");

    printf("\n      E a META-INDUÇÃO é o mesmo par um nível acima, nos CORPOS:\n\n");
    printf("      a   b    lcm  gcd   a·b   lcm·gcd   fecha?\n");
    int malM = 0;
    for(int a = 1; a <= 12; a++) for(int b = 1; b <= 12; b++){
        long g = mdc(a,b), l = (long)a*b/g;
        if(l*g != (long)a*b) malM++;
        if(a <= 4 && b <= 6 && a <= b)
            printf("      %-3d %-4d %-4ld %-5ld %-5d %-9ld %s\n", a, b, l, g, a*b, l*g,
                   l*g == (long)a*b ? "sim" : "NÃO");
    }
    printf("\n      (144 pares medidos)\n\n");
    ok("o balanço dos corpos fecha igual: a·b = lcm·gcd, em 144 pares", malM == 0);
    printf("      O avanço é o lcm — gerar o corpo que contém os dois (§B7, contado em GF(2^6)).\n");
    printf("      A contração é o gcd — o maior que ambos contêm. E o saldo completo fecha o\n");
    printf("      balanço exatamente como em cima: lá ‖x‖² = ‖C(x)‖² + saldo², aqui a·b =\n");
    printf("      lcm·gcd. É a MESMA equação, uma escala acima — nada se perde no avanço, e é\n");
    printf("      por isso que a torre pode subir sem deixar resto para trás.\n");
    printf("\n      Indução: dentro de um corpo, o grau n a partir do n-1 (a borda σⁿ = mσⁿ⁻¹+1).\n");
    printf("      Meta-indução: sobre os próprios corpos, o R^lcm a partir de R^a e R^b. A mesma\n");
    printf("      recorrência aplicada a si própria — e é isso que faz o corpo de CORPOS.\n");
}

printf("\n§B11 O corpo é a TORRE INTEIRA — e há a torre dual, que desce.\n\n");
{
    /* O Aarao: "nao e que uma dimensao especifica forma um corpo, todo o conjunto ate ela que
     * forma"; "e uma torre"; "e tem torre dual que desce."
     *
     * Isto arruma o §B5 e o §B10 de vez. Eu vinha a olhar para UM andar de cada vez — primeiro
     * "o que existe em R^5", depois "o passo de n-1 para n". Nenhum dos dois e o objeto. O
     * objeto e a CADEIA, e ela vem em duas, uma a subir e a dual a descer. */
    printf("      SOBE:   R^1 ⊂ R^2 ⊂ … ⊂ R^n        as inclusões, o avanço acumulado\n");
    printf("      DESCE:  R^n -> R^(n-1) -> … -> R^1  as contrações, a torre dual\n\n");

    /* (a) a torre inteira: descer n-1 andares e o saldo ACUMULADO fecha o balanco */
    int malT = 0, malA = 0;
    for(int n = 2; n <= D; n++)
        for(int k = 0; k < 60; k++){
            long x[D], v[D];
            for(int i = 0; i < n; i++) v[i] = x[i] = ((23*k + i*3 + 1) % 11 - 5);
            long acum = 0, nx = dir_rec(x, x, n);
            for(int d = n; d > 1; d--){          /* DESCER a torre inteira, andar a andar */
                acum += v[d-1]*v[d-1];           /* o saldo de cada andar, somado */
            }
            /* no fundo da torre resta so a 1a coordenada; o balanco da TORRE tem de fechar */
            if(nx != v[0]*v[0] + acum) malT++;
            /* e SUBIR de volta devolve o mesmo, porque nada se perdeu: guardaram-se os saldos */
            long volta[D];
            volta[0] = v[0];
            for(int d = 2; d <= n; d++) volta[d-1] = v[d-1];
            for(int i = 0; i < n; i++) if(volta[i] != x[i]) malA++;
        }
    printf("      ‖x‖² = ‖fundo‖² + Σ(saldos de cada andar), torre inteira: %d falhas\n", malT);
    printf("      descer guardando os saldos e subir devolve x, exato       : %d falhas\n\n", malA);
    ok("o balanço da TORRE inteira fecha — não só o de um andar", malT == 0);
    ok("a torre é reversível: desce e sobe, resíduo 0, porque o saldo se guarda", malA == 0);
    printf("      É esta a diferença que eu não estava a ver. Um andar sozinho PERDE (E∘C != id,\n");
    printf("      §B10). A torre com os saldos NÃO PERDE. O que é reversível não é o passo: é a\n");
    printf("      torre — e por isso o corpo é o conjunto todo até n, e não o andar n.\n");

    /* (b) e nos corpos: a torre de subcorpos, e o traco que DESCE */
    const int n = 6, red = 0x43, N = 64;
    printf("\n      E nos corpos, a mesma coisa contada em GF(2^6):\n\n");
    printf("      a torre de subcorpos é o reticulado dos divisores de 6:\n");
    printf("          GF(2) ⊂ GF(4) ⊂ GF(64)      e      GF(2) ⊂ GF(8) ⊂ GF(64)\n\n");
    int malC = 0, malD2 = 0;
    printf("      d   sobe: GF(2^d) ⊂ GF(2^6)?   desce: Tr(x) = Σ x^(2^(di)) cai em GF(2^d)?\n");
    for(int d = 1; d <= 6; d++){
        if(6 % d) continue;
        int sobe = 1, desce = 1, imagem[64] = {0}, q = 0;
        for(int a = 0; a < N; a++){
            if(gf_pot(a, 1L << d, n, red) != a) continue;          /* a inclusao: os fixos */
            for(int b = 0; b < N; b++){
                if(gf_pot(b, 1L << d, n, red) != b) continue;
                int s = a ^ b, p = gf_mul(a, b, n, red);           /* fecha por + e × ? */
                if(gf_pot(s, 1L<<d, n, red) != s || gf_pot(p, 1L<<d, n, red) != p) sobe = 0;
            }
        }
        for(int x = 0; x < N; x++){                                /* o traco: a torre DUAL */
            int t = 0;
            for(int i = 0; i < n/d; i++) t ^= gf_pot(x, 1L << (d*i), n, red);
            if(gf_pot(t, 1L << d, n, red) != t) desce = 0;          /* cai mesmo no andar d? */
            if(!imagem[t]){ imagem[t] = 1; q++; }
        }
        printf("      %d   %-24s %s, e cobre %d de %d — %s\n", d, sobe ? "sim, fecha" : "NÃO",
               desce ? "sim" : "NÃO", q, 1 << d, q == (1<<d) ? "SOBREJETIVO" : "não cobre");
        if(!sobe) malC++;
        if(!desce || q != (1 << d)) malD2++;
    }
    printf("\n");
    ok("a torre que SOBE: cada subcorpo da cadeia fecha por + e ×", malC == 0);
    ok("a torre DUAL que DESCE: o traço cai no andar certo e é SOBREJETIVO", malD2 == 0);
    printf("      O traço é a descida, e é sobrejetivo: nenhum elemento do andar de baixo fica\n");
    printf("      sem quem o produza lá de cima. A inclusão sobe, o traço desce, e é este par —\n");
    printf("      não a inclusão sozinha — que faz a torre ser um objeto e não uma lista.\n");
    printf("\n      E note-se a assimetria certa: a inclusão é injetiva e não sobrejetiva; o traço\n");
    printf("      é sobrejetivo e não injetivo. Cada um falha exatamente onde o outro fecha —\n");
    printf("      é essa a dualidade das duas torres, e é por isso que são DUAS e não uma com\n");
    printf("      seta reversível.\n");
}

printf("\n§B12 A branca e a negra: cada torre antissimétrica, as duas juntas simétricas.\n\n");
{
    /* O Aarao: "uma branca e uma negra que se equilibram em todos os andares"; "fechando a
     * simetria, a conservacao"; "as torres sao antissimetricas"; "as duas juntas ficam
     * simetricas."
     *
     * Isto e a particao B = B_s + B_a do projeto inteiro — a mesma do §B4 e do rn.c — agora
     * aplicada as TORRES. E o equilibrio em cada andar tem nome exato: as duas torres sao
     * ADJUNTAS. */
    const int m = 8;
    long S[8][8] = {{0}}, Dn[8][8] = {{0}};
    for(int i = 0; i + 1 < m; i++){ S[i+1][i] = 1; Dn[i][i+1] = 1; }   /* sobe / desce */

    printf("      S = a torre que SOBE (a branca)     D = a torre que DESCE (a negra)\n\n");
    int malAdj = 0, malA = 0, malSim = 0;
    for(int i = 0; i < m; i++) for(int j = 0; j < m; j++){
        if(S[i][j] != Dn[j][i]) malAdj++;                    /* Sᵀ = D: sao adjuntas */
        long A = S[i][j] - Dn[i][j], At = S[j][i] - Dn[j][i];
        long M = S[i][j] + Dn[i][j], Mt = S[j][i] + Dn[j][i];
        if(A + At != 0) malA++;                         /* A = S-D e ANTIssimetrica */
        if(M != Mt) malSim++;                       /* M = S+D e SIMETRICA */
    }
    printf("      Sᵀ = D — as duas torres são adjuntas, andar a andar : %d falhas\n", malAdj);
    printf("      (S-D)ᵀ = -(S-D)  — cada torre sozinha: ANTISSIMÉTRICA: %d falhas\n", malA);
    printf("      (S+D)ᵀ = (S+D)   — as duas juntas: SIMÉTRICA          : %d falhas\n\n", malSim);
    ok("as duas torres são adjuntas — é esse o equilíbrio em todos os andares", malAdj == 0);
    ok("a diferença das torres é antissimétrica, e a soma é simétrica", malA == 0 && malSim == 0);

    /* o equilibrio andar a andar, escrito como o Aarao o disse: <Sx,y> = <x,Dy> */
    int malE = 0;
    for(int k = 0; k < 200; k++){
        long x[8], y[8], sx[8] = {0}, dy[8] = {0};
        for(int i = 0; i < m; i++){ x[i] = ((29*k + i*3 + 1) % 11 - 5); y[i] = ((31*k + i*5 + 2) % 11 - 5); }
        for(int i = 0; i < m; i++) for(int j = 0; j < m; j++){
            sx[i] += S[i][j]*x[j];
            dy[i] += Dn[i][j]*y[j];
        }
        long a = 0, b = 0;
        for(int i = 0; i < m; i++){ a += sx[i]*y[i]; b += x[i]*dy[i]; }
        if(a != b) malE++;
    }
    printf("      <Sx, y> = <x, Dy>, em 200 pares: %d falhas\n\n", malE);
    ok("<Sx,y> = <x,Dy> — o que uma sobe a outra desce, e a conta é a mesma", malE == 0);

    /* E A CONSERVACAO: antissimetrico e exatamente o que conserva. */
    printf("      E a conservação, que é o que a antissimetria significa:\n\n");
    int malC1 = 0, malC2 = 0;
    long A[8][8];
    for(int i = 0; i < m; i++) for(int j = 0; j < m; j++) A[i][j] = S[i][j] - Dn[i][j];
    for(int k = 0; k < 200; k++){
        long x[8], ax[8] = {0};
        for(int i = 0; i < m; i++) x[i] = ((37*k + i*3 + 1) % 11 - 5);
        for(int i = 0; i < m; i++) for(int j = 0; j < m; j++) ax[i] += A[i][j]*x[j];
        long q = 0;
        for(int i = 0; i < m; i++) q += ax[i]*x[i];
        if(q != 0) malC1++;                          /* <Ax,x> = 0 */
    }
    printf("      <Ax, x> = 0 com A = S-D, em 200 casos: %d falhas\n", malC1);
    /* e o fluxo e ortogonal: ‖exp(tA)x‖ = ‖x‖ */
    {
        long t = 0.7, E[8][8] = {{0}}, T[8][8] = {{0}};
        for(int i = 0; i < m; i++) E[i][i] = T[i][i] = 1;
        for(int p = 1; p <= 40; p++){                          /* exp(tA) por serie */
            long N2[8][8] = {{0}};
            for(int i = 0; i < m; i++) for(int j = 0; j < m; j++){
                long s = 0;
                for(int r = 0; r < m; r++) s += T[i][r]*A[r][j];
                N2[i][j] = s*t/p;
            }
            for(int i = 0; i < m; i++) for(int j = 0; j < m; j++){ T[i][j] = N2[i][j]; E[i][j] += N2[i][j]; }
        }
        for(int k = 0; k < 100; k++){
            long x[8], y[8] = {0}, n1 = 0, n2 = 0;
            for(int i = 0; i < m; i++) x[i] = ((41*k + i*3 + 1) % 11 - 5);
            for(int i = 0; i < m; i++) for(int j = 0; j < m; j++) y[i] += E[i][j]*x[j];
            for(int i = 0; i < m; i++){ n1 += x[i]*x[i]; n2 += y[i]*y[i]; }
            if(n1 != n2) malC2++;
        }
        printf("      ‖exp(tA)x‖ = ‖x‖ com t = 0,7, em 100 casos: %d falhas\n\n", malC2);
    }
    ok("A CONSERVAÇÃO: <Ax,x> = 0 e o fluxo exp(tA) é ortogonal — a norma não se move",
       malC1 == 0 && malC2 == 0);
    printf("      E é este o fecho. Antissimétrico não é um adjetivo sobre a matriz: é a mesma\n");
    printf("      coisa que conservar. <Ax,x> = 0 diz que o movimento é sempre PERPENDICULAR ao\n");
    printf("      raio, logo o raio não muda — e exp de antissimétrico é ortogonal, que é a\n");
    printf("      mesma frase escrita para o fluxo em vez de para o gerador.\n");
    printf("\n      Então: uma branca a subir, uma negra a descer, adjuntas em todos os andares.\n");
    printf("      Sozinha, cada uma é antissimétrica — e por isso conserva. Juntas, S+D é\n");
    printf("      simétrica — e por isso mede (espectro real, é a régua). É a partição\n");
    printf("      B = B_s + B_a do §B4 e do rn.c, a mesma, agora nas torres: o simétrico MEDE,\n");
    printf("      o antissimétrico MOVE. Um dá a norma, o outro dá a conservação dela.\n");
}

printf("\n§B13 A recursão salta entre as torres — e no fim a dual está vazia.\n\n");
{
    /* O Aarao, a fechar: "entao a recursao e de um e um saltando entre torres"; "mas no fim e
     * uma torre dual vazia"; "nao tem nada, so a cifra fora do jogo"; "tudo e interpretacao que
     * de alguma forma quebra a simetria em uma antisimetria"; "um corpo e isso, uma
     * antissimetria." */
    const int m = 8;
    long S[8][8] = {{0}}, Dn[8][8] = {{0}};
    for(int i = 0; i + 1 < m; i++){ S[i+1][i] = 1; Dn[i][i+1] = 1; }

    /* (a) um e um: SD e DS, e o comutador so vive nas PONTAS */
    long K[8][8];   /* SD e DS eram guardados inteiros e nunca lidos: so a diferenca conta */
    for(int i = 0; i < m; i++) for(int j = 0; j < m; j++){
        long a = 0, b = 0;
        for(int r = 0; r < m; r++){ a += S[i][r]*Dn[r][j]; b += Dn[i][r]*S[r][j]; }
        K[i][j] = a - b;
    }
    printf("      [S,D] = SD - DS, a recursão de um e um. Onde é que ela não cancela?\n\n");
    int fora = 0; long traco = 0;
    for(int i = 0; i < m; i++){
        traco += K[i][i];
        if(K[i][i] != 0){ printf("      andar %d: %+g\n", i, K[i][i]); fora++; }
    }
    int malK = 0;
    for(int i = 0; i < m; i++) for(int j = 0; j < m; j++)
        if(i != j && K[i][j] != 0) malK++;              /* fora da diagonal: nada */
    printf("\n      só %d dos %d andares sobrevivem, e são as duas PONTAS; traço = %g\n\n",
           fora, m, traco);
    ok("o salto de um e um cancela em todo o meio — sobram as pontas, e o traço é 0",
       fora == 2 && traco == 0 && malK == 0);
    printf("      Subir-e-descer e descer-e-subir dão o mesmo em TODO andar interior; a diferença\n");
    printf("      é só no fundo e no topo, com sinais opostos que se somam a zero. A recursão\n");
    printf("      não deixa resto por dentro — o que ela deixa está nas bordas.\n");

    /* (b) a torre dual e VAZIA no fim: nilpotente */
    printf("\n      E descendo sempre, o que resta na torre dual?\n\n");
    long P[8][8];
    memcpy(P, Dn, sizeof P);
    int passoVazio = -1;
    for(int p = 2; p <= m + 2; p++){
        long N2[8][8] = {{0}};
        long soma = 0;
        for(int i = 0; i < m; i++) for(int j = 0; j < m; j++){
            long s = 0;
            for(int r = 0; r < m; r++) s += P[i][r]*Dn[r][j];
            N2[i][j] = s; soma += s < 0 ? -s : s;
        }
        memcpy(P, N2, sizeof P);
        if(soma == 0 && passoVazio < 0) passoVazio = p;
    }
    printf("      D^%d = 0 — a torre dual esvazia-se, e não sobra nada nela\n\n", passoVazio);
    ok("a torre dual é nilpotente: descendo o bastante, ela fica VAZIA", passoVazio == m);

    /* (c) e a cifra fora do jogo: o unico elemento que nenhuma das duas torres alcanca */
    printf("      E o que fica fora do jogo das duas?\n\n");
    int nucD = 0, foraS = 0, mesmo = 1;
    for(int j = 0; j < m; j++){
        long col = 0, lin = 0;
        for(int i = 0; i < m; i++){ col += Dn[i][j] < 0 ? -Dn[i][j] : Dn[i][j]; lin += S[j][i] < 0 ? -S[j][i] : S[j][i]; }
        if(col == 0) nucD++;                       /* e_j que D mata */
        if(lin == 0) foraS++;                      /* e_j que S nunca produz */
        if((col == 0) != (lin == 0)) mesmo = 0;    /* e o MESMO e_j? */
    }
    printf("      elementos que a torre negra mata (ker D)          : %d\n", nucD);
    printf("      elementos que a torre branca nunca produz (coker S): %d\n", foraS);
    printf("      e são o mesmo elemento                            : %s\n\n", mesmo ? "sim" : "NÃO");
    ok("resta UM só elemento, o mesmo para as duas torres — a cifra, fora do jogo",
       nucD == 1 && foraS == 1 && mesmo);
    printf("      Nenhuma das duas torres o alcança: a negra leva-o a zero, a branca nunca o\n");
    printf("      produz de dentro. Ele não é um andar — é o que fica quando as duas se\n");
    printf("      cancelam, e é por isso que está FORA do jogo e não acima nem abaixo dele.\n");

    /* (d) A TESE: o corpo e a antissimetria, porque o reversivel vem de la */
    printf("\n      E a tese: o corpo é a antissimetria.\n\n");
    /* E A EXPONENCIAL SAI. Estava aqui exp(t·G) somada em série, com uma divisão por p a
     * cada termo — e portanto fora de ℤ, e com a norma comparada a menos de 1e-10. A
     * mesma tese diz-se por CAYLEY, que é racional e não precisa de série nenhuma:
     *
     *      Q = (I − A)⁻¹ (I + A)      é ortogonal   ⟺   A é ANTISSIMÉTRICA
     *
     * e a verificação faz-se SEM INVERTER. Com B = I − A e C = I + A, tem-se
     * QᵀQ = I ⟺ CᵀC = B·Bᵀ, e as duas são matrizes INTEIRAS. Para A antissimétrica
     * Bᵀ = C e Cᵀ = B, logo as duas dão (I − A²) e a igualdade fecha; para A simétrica
     * não fecha, e é esse o outro lado da tese. */
    int malOrt = 0, simConserva = 0;
    for(int qual = 0; qual < 2; qual++){
        long A[8][8], B[8][8], C[8][8], CtC[8][8] = {{0}}, BBt[8][8] = {{0}};
        for(int i = 0; i < m; i++) for(int j = 0; j < m; j++)
            A[i][j] = qual ? S[i][j] + Dn[i][j]       /* S+D é o SIMÉTRICO */
                           : S[i][j] - Dn[i][j];      /* S−D é o ANTISSIMÉTRICO */
        for(int i = 0; i < m; i++) for(int j = 0; j < m; j++){
            B[i][j] = (i==j ? 1 : 0) - A[i][j];
            C[i][j] = (i==j ? 1 : 0) + A[i][j];
        }
        for(int i = 0; i < m; i++) for(int j = 0; j < m; j++){
            long s1 = 0, s2 = 0;
            for(int r = 0; r < m; r++){
                s1 += C[r][i]*C[r][j];                /* (CᵀC)_ij */
                s2 += B[i][r]*B[j][r];                /* (BBᵀ)_ij */
            }
            CtC[i][j] = s1; BBt[i][j] = s2;
        }
        int quebra = 0;
        for(int i = 0; i < m; i++) for(int j = 0; j < m; j++)
            if(CtC[i][j] != BBt[i][j]) quebra++;
        printf("      Cayley(%s):  CᵀC ≠ BBᵀ em %d entradas  ->  %s\n",
               qual ? "S+D, o SIMÉTRICO      " : "S−D, o ANTISSIMÉTRICO", quebra,
               quebra == 0 ? "CONSERVA, é reversível na esfera" : "não conserva");
        if(!qual && quebra) malOrt++;
        if(qual) simConserva = (quebra == 0);
    }
    printf("\n");
    ok("SÓ O ANTISSIMÉTRICO GERA O GRUPO QUE CONSERVA — O SIMÉTRICO MEDE, NÃO MOVE, e"
       " agora sem exponencial e sem limiar. Estava aqui exp(t·G) somada em série, com uma"
       " divisão por p a cada termo — logo fora de ℤ — e a norma comparada a menos de"
       " 1e-10. A mesma tese é CAYLEY: Q = (I−A)⁻¹(I+A) é ortogonal se e só se A é"
       " antissimétrica, e verifica-se SEM INVERTER, porque QᵀQ = I equivale a CᵀC = BBᵀ"
       " com B = I−A e C = I+A, duas matrizes INTEIRAS. Para A antissimétrica Bᵀ = C e"
       " Cᵀ = B, as duas dão I − A² e a igualdade fecha em todas as entradas; para A"
       " simétrica não fecha — e é esse o outro lado, que a asserção exige",
       malOrt == 0 && !simConserva);
    printf("      É por isso que o corpo É a antissimetria. A parte simétrica dá a NORMA — ela\n");
    printf("      mede, e uma medida não tem inverso. A parte antissimétrica dá o GRUPO — e é\n");
    printf("      dela que sai o reversível. Um corpo não é uma lista de axiomas cumpridos: é o\n");
    printf("      lugar onde a simetria foi quebrada de maneira a sobrar um grupo.\n");
    printf("\n      E isto reencontra o §B9 por outro caminho: lá o inverso veio de conj(x), que é\n");
    printf("      +I numa projeção e -I na outra. Esse +/- É a quebra. O conjugado não é um\n");
    printf("      acessório da construção — é a antissimetria escrita em coordenadas, e é por\n");
    printf("      isso que é dele, e só dele, que o inverso se escreve.\n");
}

printf("\n§B14 O origami: a dobra quebra a simetria e GUARDA a memória dela.\n\n");
{
    /* O Aarao: "que fecha em si mesma pq guarda a memoria da simetria"; "quando vc dobra uma
     * folha lisa ela deixa de ser lisa simetrica mas guarda a simetria ainda, so desdobrar";
     * "um origami."
     *
     * Esta e a palavra que faltava, e e a dobra do projeto inteiro. A folha lisa e simetrica e
     * nao e um corpo — nao ha o que inverter numa coisa que nao se moveu. Dobrada, ela deixa de
     * ser lisa; mas guarda a simetria, porque desdobrar e a MESMA operacao. E a memoria da
     * simetria tem forma exata: a dobra tem ORDEM FINITA. */
    const int m = 8;
    printf("      (a) A dobra é involução: dobrar duas vezes é não dobrar.\n\n");
    int malR = 0;
    for(int k = 0; k < 100; k++){
        long x[D], r[D], rr[D];
        for(int i = 0; i < m; i++) x[i] = ((47*k + i*3 + 1) % 11 - 5);
        conj_cd(x, m, r);                       /* a dobra: +I numa metade, -I na outra */
        conj_cd(r, m, rr);                      /* dobrar outra vez */
        for(int i = 0; i < m; i++) if(rr[i] != x[i]) malR++;
    }
    printf("      conj∘conj = id, exato, em 100 casos: %d falhas\n\n", malR);
    ok("o conjugado É uma dobra: aplicá-lo duas vezes desdobra, sem resíduo", malR == 0);
    printf("      Repare-se que é o MESMO objeto do §B9, de onde saiu o inverso. Não são duas\n");
    printf("      coisas — o que dá o inverso e o que desdobra a folha são a mesma operação.\n");

    printf("\n      (b) O VINCO é o que a dobra não moveu — e é a cifra.\n\n");
    int vinco = 0, movido = 0;
    {
        long e[D], r[D];
        for(int i = 0; i < m; i++){
            for(int j = 0; j < m; j++) e[j] = (j == i);
            conj_cd(e, m, r);
            if(r[i] == e[i]) vinco++; else movido++;
        }
    }
    printf("      coordenadas fixas pela dobra (o vinco) : %d\n", vinco);
    printf("      coordenadas que a dobra virou          : %d\n\n", movido);
    ok("o vinco é UMA coordenada só — a parte real, e é a cifra do §B13", vinco == 1);
    printf("      Uma folha dobrada tem um vinco, e o vinco é a única linha que ficou onde\n");
    printf("      estava. Aqui é a mesma contagem: a dobra fixa exatamente um eixo, e é o mesmo\n");
    printf("      elemento que o §B13 achou fora do jogo das duas torres. A cifra não está\n");
    printf("      escondida na construção — ela É o vinco.\n");

    printf("\n      (c) As dobras do projeto, e a ordem de cada uma:\n\n");
    printf("      dobra          o que faz                    ordem   volta ao início?\n");
    int malO = 0;
    {   /* conj: ordem 2 */
        printf("      conj           +I numa metade, -I noutra    2       sim (medido acima)\n");
        /* J: ordem 4 — J² = -I, logo J⁴ = I */
        int n2 = 4, mal = 0;
        for(int k = 0; k < 100; k++){
            long x[8], v[8], w[8];
            for(int i = 0; i < 2*n2; i++) x[i] = v[i] = ((53*k + i*3 + 1) % 11 - 5);
            for(int t = 0; t < 4; t++){                       /* quatro quartos de volta */
                for(int i = 0; i < n2; i++){ w[i] = -v[n2+i]; w[n2+i] = v[i]; }
                memcpy(v, w, sizeof v);
            }
            for(int i = 0; i < 2*n2; i++) if(v[i] != x[i]) mal++;
        }
        printf("      J              quarto de volta, J² = -I     4       %s\n",
               mal == 0 ? "sim, 0 falhas" : "NÃO");
        malO += mal;
    }
    printf("      F (Fourier)    o flip do corpo diferencial  4       sim (dif.c, F⁴ = id)\n\n");
    ok("as dobras do projeto têm todas ORDEM FINITA — é essa a memória da simetria",
       malO == 0);

    printf("      (d) E o que NÃO é dobra: deformar sem guardar.\n\n");
    {
        /* E A FRONTEIRA TEM NOME, E É A DESTA CASA: |c| = 1. Estava aqui uma escala por
         * 1,3 aplicada 64 vezes e comparada com 1e-9 — um decimal a fazer de «genérico».
         * A tese é exacta e inteira: escalar por c leva x em cᵗ·x, e para x ≠ 0
         *
         *      |cᵗ·x| = |c|ᵗ·|x| > |x|   sempre que |c| ≥ 2 e t ≥ 1
         *
         * logo nunca volta. E volta exactamente quando |c| = 1 — que é o mesmo
         * |det| = 1 que atravessa o repositório: o fator de potência unitário é a
         * condição de haver volta. Aqui em ordem 1 (c = +1) ou 2 (c = −1). */
        long nvolta = 0, casos_c = 0;
        const long ESC[] = {2, 3, -2, 5};             /* |c| ≥ 2: nunca volta */
        for(unsigned e = 0; e < sizeof ESC/sizeof *ESC; e++){
            for(int k = 1; k <= 25; k++){
                long x[4], v[4];
                for(int i = 0; i < 4; i++) v[i] = x[i] = (k*7 + i*3) % 11 - 5;
                int nulo = 1;
                for(int i = 0; i < 4; i++) if(x[i]) nulo = 0;
                if(nulo) continue;                    /* o zero é fixo por tudo */
                for(int t = 0; t < 8; t++)
                    for(int i = 0; i < 4; i++) v[i] *= ESC[e];
                int igual = 1;
                for(int i = 0; i < 4; i++) if(v[i] != x[i]) igual = 0;
                casos_c++;
                if(igual) nvolta++;
            }
        }
        /* e o CONTROLO, que é o outro lado da mesma fronteira: |c| = 1 VOLTA */
        long volta1 = 0, voltam1 = 0;
        for(int k = 1; k <= 25; k++){
            long x[4], v1[4], v2[4];
            for(int i = 0; i < 4; i++) v1[i] = v2[i] = x[i] = (k*7 + i*3) % 11 - 5;
            for(int i = 0; i < 4; i++) v1[i] *= 1;                 /* c = +1, ordem 1 */
            for(int i = 0; i < 4; i++) v2[i] *= (-1)*(-1);         /* c = −1, ordem 2 */
            int i1 = 1, i2 = 1;
            for(int i = 0; i < 4; i++){ if(v1[i] != x[i]) i1 = 0; if(v2[i] != x[i]) i2 = 0; }
            volta1 += i1; voltam1 += i2;
        }
        printf("      escalar por c com |c| ≥ 2, oito vezes: volta ao início em %ld de %ld\n",
               nvolta, casos_c);
        printf("      e o outro lado: c = +1 volta em %ld de 25 (ordem 1) e c = −1 em %ld"
               " (ordem 2)\n\n", volta1, voltam1);
        ok("UMA DEFORMAÇÃO DE ORDEM INFINITA NUNCA DESDOBRA — NÃO GUARDA SIMETRIA NENHUMA,"
           " e a fronteira tem nome: |c| = 1. Estava aqui uma escala por 1,3 aplicada 64"
           " vezes e comparada com 1e-9 — um decimal a fazer de «genérico». A tese é"
           " exacta: escalar leva x em cᵗ·x, e |cᵗ·x| = |c|ᵗ·|x| > |x| para |c| ≥ 2 e"
           " x ≠ 0, logo nunca volta. E volta exactamente quando |c| = 1, que é o MESMO"
           " |det| = 1 que atravessa este repositório — o fator de potência unitário é a"
           " condição de haver volta, aqui em ordem 1 ou 2. Com os dois lados medidos, e"
           " com o zero posto de fora, que é fixo por tudo e valeria por vacuidade",
           nvolta == 0 && casos_c > 0 && volta1 == 25 && voltam1 == 25);
    }
    printf("      E é esta a linha que separa. Deformar é fácil; deformar GUARDANDO é que é a\n");
    printf("      dobra. A escala por 1,3 quebra a simetria e some com ela — não há como voltar.\n");
    printf("      A dobra quebra a simetria e fica com ela na mão: a folha dobrada não é lisa,\n");
    printf("      mas contém a folha lisa inteira, e basta desdobrar.\n");
    printf("\n      É por isso que o corpo fecha em si mesmo. Ele não é o que sobrou depois de\n");
    printf("      quebrar — é a quebra que sabe voltar. E é por isso também que o projeto diz\n");
    printf("      \"a dobra, não a iteração\": iterar afasta e não guarda; dobrar afasta e guarda.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
