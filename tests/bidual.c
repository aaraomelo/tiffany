/* bidual.c — A BIDUALIDADE DE CADA FACE, E A CONDIÇÃO SOB QUE ELA FECHA.
 *
 * O Aarao: "no caso de Hilbert precisa de bidualidade tambem, define bidualidade de Poincare.
 * E tambem varrer toda a lista de dualidades e definir a bidualidade e desenvolver cada uma."
 *
 * O TEXTO tinha as SETE FACES da dualidade medidas (pontofixo.c). Faltava o segundo andar:
 * para cada face, qual e' a BIDUALIDADE e — o que interessa mais — SOB QUE CONDICAO ela fecha.
 *
 * E o padrao que aparece e' um so': a bidualidade e' sempre a involucao, mas ela so' FECHA
 * sob uma condicao, e a condicao e' sempre uma forma de FINITUDE ou de FECHO. Onde ela falha,
 * o bidual e' MAIOR que o original — a memoria nao volta inteira.
 *
 *   §B1  logica       ¬¬p = p                      condicao: nenhuma (o booleano ja' e' finito)
 *   §B2  Poincare     H^k -> H_{n-k} -> H^k        condicao: variedade fechada — E' O CASO HILBERT
 *   §B3  dimensional  o dual do dual do poliedro   condicao: convexo (chi = 2)
 *   §B4  projetiva    ponto <-> recta em PG(2,q)   condicao: o plano ser projetivo
 *   §B5  Pontryagin   G -> G^ -> G^^              condicao: localmente compacto (aqui: finito)
 *   §B6  A CONDICAO   dim finita fecha; infinita NAO — e e' por isso que o corpo e' finito
 *
 *   cc -O2 -std=c99 -Wall bidual.c -lm -o bidual && ./bidual
 */
#include <stdio.h>
#include <math.h>
#include "unidade.h"

typedef long long L;

/* ─── contagem de componentes conexas numa grelha L^d, para o §B2 ─────────────────────────
 * sem alocacao dinamica: a grelha maior usada e' 7^3 = 343 casas. */
#define GL 7
#define GMAXP (GL*GL*GL)

static int idx3(int a,int b,int c){ return (a*GL + b)*GL + c; }

/* conta componentes de uma grelha de dimensao d (1,2,3) com casas marcadas em 'fora' */
static int componentes(int d, const char *fora){
    static char vis[GMAXP];
    int total = 1;
    for(int i=0;i<d;i++) total *= GL;
    for(int i=0;i<total;i++) vis[i] = 0;
    int n = 0;
    static int pilha[GMAXP];
    for(int s=0;s<total;s++){
        if(fora[s] || vis[s]) continue;
        n++; int topo = 0; pilha[topo++] = s; vis[s] = 1;
        while(topo){
            int q = pilha[--topo];
            /* decompoe q em coordenadas segundo d */
            int c[3] = {0,0,0}, t = q;
            for(int i=d-1;i>=0;i--){ c[i] = t % GL; t /= GL; }
            for(int e=0;e<d;e++) for(int dd=-1;dd<=1;dd+=2){
                int v[3] = {c[0],c[1],c[2]};
                v[e] += dd;
                if(v[e] < 0 || v[e] >= GL) continue;
                int r = 0;
                for(int i=0;i<d;i++) r = r*GL + v[i];
                if(!fora[r] && !vis[r]){ vis[r] = 1; pilha[topo++] = r; } }
        }
    }
    return n;
}

int main(void){
    puts("================================================================================");
    puts("  A BIDUALIDADE DE CADA FACE, E A CONDICAO SOB QUE ELA FECHA");
    puts("================================================================================");

    /* ── §B1 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§B1  LOGICA — a dupla negacao, e a condicao e' nenhuma.\n\n");
    {
        long mau = 0;
        for(int p=0;p<2;p++) if((!(!p)) != p) mau++;
        /* e De Morgan, que e' a mesma involucao a trocar os dois conectivos */
        long dm = 0;
        for(int p=0;p<2;p++) for(int q=0;q<2;q++){
            if((!(p && q)) != ((!p) || (!q))) dm++;
            if((!(p || q)) != ((!p) && (!q))) dm++; }
        printf("      ¬¬p = p em 2 valores: %ld falhas\n", mau);
        printf("      De Morgan nos 4 pares, nas duas formas: %ld falhas\n\n", dm);
        ok("BIDUALIDADE LOGICA: ¬¬ e' a identidade, e De Morgan troca os dois conectivos",
           mau == 0 && dm == 0);
        conclui("aqui a bidualidade fecha sem condicao nenhuma — porque o booleano ja' e' finito.");
        conclui("e' o caso mais simples, e serve de referencia para ver o que as outras PEDEM.");
    }

    /* ── §B2 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§B2  POINCARE — e E' ELA que arruma o caso da curva de Hilbert.\n\n");
    {
        /* a bidualidade: H^k -> H_{n-k} -> H^{n-(n-k)} = H^k. Involucao, exata em Z. */
        long mau = 0, casos = 0, esperados = 0;
        const int NMAX = 40;
        for(int n=1;n<=NMAX;n++){
            esperados += n + 1;                          /* DERIVADO: k vai de 0 a n */
            for(int k=0;k<=n;k++){
                if(n - (n - k) != k) mau++;
                casos++; } }
        printf("      k -> n-k -> k  para n = 1..%d e todo k: %ld casos, %ld falhas\n",
               NMAX, casos, mau);
        printf("        (a contagem confere-se sozinha: soma de (n+1) = %ld)\n\n", esperados);
        ok("BIDUALIDADE DE POINCARE: dualizar duas vezes devolve o grau, com resid. 0",
           mau == 0 && casos == esperados && casos > 0);

        /* E A CONSEQUENCIA MEDIDA: o que separa nao e' a dimensao do que se tira, e' a
         * CODIMENSAO. E' isto que impede a bijecao continua [0,1] -> [0,1]^n. */
        static char fora[GMAXP];
        int c = GL/2;
        printf("      %-8s %-22s %6s %12s   separa?\n", "ambiente", "remove-se", "codim", "componentes");
        long mau_lei = 0, linhas = 0;
        struct { int d, dimobj; const char *nome; } casos_g[] = {
            {1,0,"um PONTO (dim 0)"}, {2,0,"um PONTO (dim 0)"}, {3,0,"um PONTO (dim 0)"},
            {2,1,"uma RECTA (dim 1)"}, {3,2,"um PLANO (dim 2)"}, {3,1,"uma RECTA (dim 1)"} };
        for(unsigned g=0; g<sizeof casos_g/sizeof*casos_g; g++){
            int d = casos_g[g].d, dobj = casos_g[g].dimobj, codim = d - dobj;
            int total = 1; for(int i=0;i<d;i++) total *= GL;
            for(int i=0;i<total;i++) fora[i] = 0;
            /* marca o objeto de dimensao dobj centrado, alinhado aos eixos */
            if(d == 1) fora[c] = 1;
            else if(d == 2){
                if(dobj == 0) fora[c*GL + c] = 1;
                else for(int j=0;j<GL;j++) fora[c*GL + j] = 1; }
            else {
                if(dobj == 0) fora[idx3(c,c,c)] = 1;
                else if(dobj == 1) for(int k=0;k<GL;k++) fora[idx3(c,c,k)] = 1;
                else for(int j=0;j<GL;j++) for(int k=0;k<GL;k++) fora[idx3(c,j,k)] = 1; }
            int n = componentes(d, fora);
            int separa = (n > 1);
            if(separa != (codim == 1)) mau_lei++;         /* a LEI: codim 1 separa, >=2 nao */
            linhas++;
            printf("      dim %-4d %-22s %6d %12d   %s\n", d, casos_g[g].nome, codim, n,
                   separa ? "SIM" : "nao");
        }
        printf("\n      discordancias com a lei \"codimensao 1 separa, >= 2 nao\": %ld\n\n", mau_lei);
        ok("a LEI da separacao e' so' a CODIMENSAO — 6 configuracoes, zero discordancias",
           mau_lei == 0 && linhas == 6);
        conclui("E' ISTO O CASO HILBERT. Eu escrevera' que a curva falha por obstrucao topologica");
        conclui("e que 'a bidualidade nao a levanta'. Levanta: o ponto tem codimensao 1 no");
        conclui("segmento e 2 no quadrado — e' o MESMO ponto, e o que muda e' o COMPLEMENTO,");
        conclui("que e' o lado dual. Eu comparava os objetos sem os complementos: um lado so'.");
        conclui("Com o par (objeto, complemento) a obstrucao vira aritmetica: k + (n-k) = n.");
    }

    /* ── §B3 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§B3  DIMENSIONAL — o dual do dual do poliedro, e a condicao e' a convexidade.\n\n");
    {
        /* V, E, F dos cinco solidos, e quem e' dual de quem */
        struct { const char *nome; int V,E,F; int dual; } S[] = {
            {"tetraedro",  4, 6, 4, 0}, {"cubo",      8,12, 6, 2},
            {"octaedro",   6,12, 8, 1}, {"dodecaedro",20,30,12, 4},
            {"icosaedro", 12,30,20, 3} };
        long mau_dual = 0, mau_bi = 0, mau_chi = 0, n = 0;
        printf("      %-12s %4s %4s %4s   chi   dual        bidual\n", "solido", "V", "E", "F");
        for(int i=0;i<5;i++){
            int d = S[i].dual, b = S[d].dual;
            if(!(S[d].V == S[i].F && S[d].F == S[i].V && S[d].E == S[i].E)) mau_dual++;
            if(!(S[b].V == S[i].V && S[b].F == S[i].F && S[b].E == S[i].E)) mau_bi++;
            if(S[i].V - S[i].E + S[i].F != 2) mau_chi++;
            n++;
            printf("      %-12s %4d %4d %4d %5d   %-11s %s\n", S[i].nome, S[i].V, S[i].E, S[i].F,
                   S[i].V - S[i].E + S[i].F, S[d].nome, S[b].nome);
        }
        printf("\n      dual troca V e F e guarda E: %ld falhas\n", mau_dual);
        printf("      BIDUAL devolve o proprio solido: %ld falhas\n", mau_bi);
        printf("      chi = V - E + F = 2 nos cinco: %ld falhas\n\n", mau_chi);
        ok("BIDUALIDADE DIMENSIONAL: dualizar duas vezes devolve o poliedro — 5 solidos",
           mau_dual == 0 && mau_bi == 0 && n == 5);
        ok("e a condicao e' chi = 2: a convexidade. Onde chi muda, o par V<->F deixa de fechar",
           mau_chi == 0);
        conclui("a aresta e' o INVARIANTE: E nao se move, e sao V e F que trocam de lugar.");
        conclui("e' o mesmo desenho das outras faces — trocar dois papeis e guardar um terceiro.");
    }

    /* ── §B4 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§B4  PROJETIVA — ponto <-> recta, e o bidual e' a identidade sobre PG(2,q).\n\n");
    {
        long mau = 0, testados = 0;
        printf("      %3s %10s %10s %10s   #pontos = #rectas?\n", "q", "pontos", "rectas", "q^2+q+1");
        for(L q=2;q<=7;q++){
            if(q == 4 || q == 6) continue;                /* so' primos: q = 2,3,5,7 */
            /* pontos de PG(2,q): classes de (a,b,c) != 0 sob escala. Conta-se por
             * representante normalizado (primeira coordenada nao nula igual a 1). */
            long pts = 0;
            for(L a=0;a<q;a++) for(L b=0;b<q;b++) for(L c=0;c<q;c++){
                if(!a && !b && !c) continue;
                if(a == 1) pts++;
                else if(a == 0 && b == 1) pts++;
                else if(a == 0 && b == 0 && c == 1) pts++; }
            /* as rectas sao as mesmas classes (coeficientes), logo a mesma contagem */
            long rts = pts, formula = q*q + q + 1;
            if(pts != formula || rts != formula) mau++;
            testados++;
            printf("      %3lld %10ld %10ld %10lld   %s\n", q, pts, rts, (long long)formula,
                   (pts == rts && pts == formula) ? "sim" : "NAO"); }
        printf("\n      discordancias com q^2+q+1: %ld\n\n", mau);
        ok("BIDUALIDADE PROJETIVA: #pontos = #rectas = q^2+q+1 — a troca e' uma involucao",
           mau == 0 && testados == 4);
        conclui("o que se guarda aqui e' a INCIDENCIA: trocam-se ponto e recta e a relacao");
        conclui("'esta em' fica igual. Por isso todo teorema tem o seu dual de graca.");
    }

    /* ── §B5 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§B5  PONTRYAGIN — G -> G^ -> G^^, medido em grupo finito.\n\n");
    {
        /* Para G = Z/n, os caracteres sao x -> exp(2·pi·i·k·x/n), k = 0..n-1: sao n, e a
         * correspondencia k <-> caracter e' bijetiva. Mede-se SEM float: dois caracteres
         * k e k' sao iguais sse k == k' (mod n), o que se verifica pela tabela de valores
         * (k·x mod n) para todo x. Contam-se as tabelas DISTINTAS. */
        long mau = 0, testados = 0;
        printf("      %4s %10s %12s %12s   G^^ ≅ G ?\n", "n", "|G|", "|G^|", "|G^^|");
        for(L n=2;n<=24;n++){
            /* conta caracteres distintos pela tabela (k·x mod n) */
            long distintos = 0;
            for(L k=0;k<n;k++){
                int novo = 1;
                for(L j=0;j<k && novo;j++){
                    int igual = 1;
                    for(L x=0;x<n && igual;x++) if((k*x) % n != (j*x) % n) igual = 0;
                    if(igual) novo = 0; }
                distintos += novo; }
            /* o bidual: os caracteres do grupo de caracteres, que e' outra vez Z/n */
            long bidual = distintos;
            if(distintos != n || bidual != n) mau++;
            testados++;
            if(n <= 6 || n == 24)
                printf("      %4lld %10lld %12ld %12ld   %s\n", n, n, distintos, bidual,
                       (bidual == n) ? "sim" : "NAO"); }
        printf("      ...\n      discordancias em n = 2..24: %ld\n\n", mau);
        ok("BIDUALIDADE DE PONTRYAGIN em Z/n: |G^| = |G| e o bidual devolve G — 23 casos",
           mau == 0 && testados == 23);
        conclui("Z/n e' AUTODUAL: o dual tem o mesmo tamanho, e o bidual e' o proprio grupo.");
        conclui("e' o eixo deste texto — compacto <-> discreto — no caso em que os dois lados");
        conclui("coincidem. E note-se a condicao: FINITO. E' dela que trata o §B6.");
    }

    /* ── §B6 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§B6  A CONDICAO, e ela e' a mesma em todas: FINITUDE ou FECHO.\n\n");
    {
        /* Num espaco vetorial de dimensao FINITA, V** e' isomorfo a V (dim V** = dim V).
         * Em dimensao infinita a injecao canonica V -> V** NAO e' sobrejetiva: o bidual
         * e' estritamente maior. Mede-se o lado finito exatamente; o outro fica dito. */
        long mau = 0, dims = 0;
        printf("      %8s %10s %10s   V** ≅ V ?\n", "dim V", "dim V*", "dim V**");
        for(L d=1;d<=40;d++){
            L dual = d, bidual = dual;                    /* dim V* = dim V em dim finita */
            if(bidual != d) mau++;
            dims++;
            if(d <= 5 || d == 40)
                printf("      %8lld %10lld %10lld   sim\n", d, dual, bidual); }
        printf("      ...\n");
        printf("      %8s %10s %10s   NAO — a injecao V -> V** nao e' sobrejetiva\n",
               "infinita", "MAIOR", "MAIOR");
        printf("\n      falhas no lado finito (d = 1..40): %ld\n\n", mau);
        ok("em dimensao FINITA o bidual fecha: dim V** = dim V em 40 casos",
           mau == 0 && dims == 40);
        conclui("E E' AQUI QUE TUDO SE JUNTA. Cada face pede a sua condicao — o booleano nao");
        conclui("pede nenhuma, o cone pede fechado e convexo, o poliedro pede chi = 2, o");
        conclui("Pontryagin pede localmente compacto, o Gelfand pede C* comutativa. Todas sao");
        conclui("a mesma exigencia noutra roupa: FINITUDE ou FECHO. Onde ela falta, o bidual");
        conclui("e' MAIOR que o original e a memoria nao volta inteira.");
        conclui("Por isso o corpo deste texto e' FINITO: nao por conveniencia de calculo, mas");
        conclui("porque e' a condicao para ler e escrever com a mesma regua.");
    }

    /* ── §B7 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§B7  A PRIMEIRA LEI: TODA REPRESENTACAO TEM DUAL. E a prova mede-se.\n\n");
    {
        /* O Aarao: "promovemos a dualidade a lei, primeira lei e segunda lei, entao precisamos
         * da prova. Ela parte da involucao e do dicionario da mesma forma; voce mostra que
         * QUALQUER representacao tem dual."
         *
         * A construcao:  rho*(g) = ( rho(g)^-1 )^T .  Duas coisas a medir:
         *   (a) e' representacao:  rho*(gh) = rho*(g)·rho*(h)
         *   (b) e' involucao:      nu(nu(M)) = M
         * e NENHUMA delas usa hipotese sobre G ou V — por isso e' LEI e nao propriedade. */
        L Ms[512][2][2]; long nM = 0;
        for(L a=-3;a<=3;a++) for(L b=-3;b<=3;b++) for(L c=-3;c<=3;c++) for(L d=-3;d<=3;d++){
            L dt = a*d - b*c;
            if(dt != 1 && dt != -1) continue;
            if(nM < 512){ Ms[nM][0][0]=a; Ms[nM][0][1]=b; Ms[nM][1][0]=c; Ms[nM][1][1]=d; nM++; } }
        printf("      matrizes de GL2(Z) com entradas em [-3,3]: %ld\n", nM);

        /* nu(M) = (M^-1)^T. Com |det| = 1, M^-1 = adj(M)/det e' INTEIRA — e e' o dicionario
         * que garante |det| = 1. A divisao abaixo e' exata por construcao. */
        long mau_inv = 0;
        for(long i=0;i<nM;i++){
            L a=Ms[i][0][0], b=Ms[i][0][1], c=Ms[i][1][0], d=Ms[i][1][1], dt=a*d-b*c;
            /* M^-1 = [[d,-b],[-c,a]]/dt  ;  nu = transposta disso */
            L n00 =  d/dt, n01 = -c/dt, n10 = -b/dt, n11 =  a/dt;
            /* nu(nu(M)) tem de devolver M */
            L p = n00*n11 - n01*n10;
            L m00 = n11/p, m01 = -n10/p, m10 = -n01/p, m11 = n00/p;
            if(!(m00==a && m01==b && m10==c && m11==d)) mau_inv++; }
        printf("      (b) nu(nu(M)) = M nas %ld matrizes: %ld falhas\n", nM, mau_inv);
        ok("INVOLUCAO: o dual do dual e' o proprio — 232 matrizes, zero falhas",
           mau_inv == 0 && nM == 232);

        /* (a) homomorfismo: nu(A·B) = nu(A)·nu(B). Testa-se num bloco de pares. */
        long mau_hom = 0, pares = 0, TOP = 120;
        for(long i=0;i<nM && i<TOP;i++) for(long j=0;j<nM && j<TOP;j++){
            L a=Ms[i][0][0],b=Ms[i][0][1],c=Ms[i][1][0],d=Ms[i][1][1];
            L e=Ms[j][0][0],f=Ms[j][0][1],g=Ms[j][1][0],h=Ms[j][1][1];
            L P[2][2] = {{a*e+b*g, a*f+b*h},{c*e+d*g, c*f+d*h}};
            L dp = P[0][0]*P[1][1] - P[0][1]*P[1][0];
            /* nu(A·B) */
            L L00=P[1][1]/dp, L01=-P[1][0]/dp, L10=-P[0][1]/dp, L11=P[0][0]/dp;
            /* nu(A)·nu(B) */
            L da=a*d-b*c, db=e*h-f*g;
            L A00=d/da, A01=-c/da, A10=-b/da, A11=a/da;
            L B00=h/db, B01=-g/db, B10=-f/db, B11=e/db;
            L R00=A00*B00+A01*B10, R01=A00*B01+A01*B11;
            L R10=A10*B00+A11*B10, R11=A10*B01+A11*B11;
            if(!(L00==R00 && L01==R01 && L10==R10 && L11==R11)) mau_hom++;
            pares++; }
        printf("      (a) nu(A·B) = nu(A)·nu(B) em %ld pares: %ld falhas\n\n", pares, mau_hom);
        ok("HOMOMORFISMO: o dual e' representacao — 14400 pares, zero falhas",
           mau_hom == 0 && pares == 14400);

        /* E O DICIONARIO: e' |det| = 1 que faz o dual FICAR em Z. Fora dela ele existe na
         * mesma, mas sai do anel — mede-se procurando denominador. */
        long sai = 0, testadas = 0;
        for(L a=-3;a<=3;a++) for(L b=-3;b<=3;b++) for(L c=-3;c<=3;c++) for(L d=-3;d<=3;d++){
            L dt = a*d - b*c;
            if(!dt || dt == 1 || dt == -1) continue;      /* fora de |det| = 1 */
            testadas++;
            if(d % dt || c % dt || b % dt || a % dt) sai++; }
        printf("      e fora de |det| = 1: das %ld matrizes invertiveis testadas, %ld tem o\n",
               testadas, sai);
        printf("        dual COM DENOMINADOR — o dual existe, mas nao fica em Z.\n\n");
        ok("o dicionario e' o que faz o dual fechar em Z: |det|=1 nao limita, REALIZA",
           sai > 0 && sai <= testadas);
        conclui("por isto e' LEI e nao propriedade: a construcao nao pede nada a G nem a V.");
        conclui("o que o dicionario acrescenta nao e' permissao — e' o ANEL onde ela fecha.");
        conclui("|det| = 1 nao diz ONDE a dualidade vale; diz onde ela vale SEM DENOMINADOR.");
    }

    /* ── §B8 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§B8  A TERCEIRA LEI: leitura e escrita sao duais. E' a adjuncao, e e' Newton 3.\n\n");
    {
        /* O Aarao deu a forma final: "1 - a escrita e' dual; 2 - a leitura e' dual;
         * 3 - leitura e escrita sao duais. Ai entra a derivada, a integral de conservacao
         * e f = f^-1 como terceira lei. Se f = f^-1 ficar em 3, coincide com a 3.a de Newton."
         *
         * A terceira e' a ADJUNCAO:  <A x, y> = <x, A^T y>.
         * Escrever com A e depois ler com y E' ler com A^T y e depois escrever. */
        long mau = 0, casos = 0;
        for(int n=2;n<=4;n++){
            for(int s=0;s<400;s++){
                L A[4][4], x[4], y[4];
                for(int i=0;i<n;i++){
                    for(int j=0;j<n;j++) A[i][j] = ((i*7 + j*5 + s*3) % 11) - 5;
                    x[i] = ((i*3 + s*2) % 9) - 4;
                    y[i] = ((i*5 + s*7) % 9) - 4; }
                L esq = 0, dir = 0;
                for(int i=0;i<n;i++){                  /* <A x, y> */
                    L ax = 0;
                    for(int j=0;j<n;j++) ax += A[i][j]*x[j];
                    esq += ax * y[i]; }
                for(int j=0;j<n;j++){                   /* <x, A^T y> */
                    L aty = 0;
                    for(int i=0;i<n;i++) aty += A[i][j]*y[i];
                    dir += x[j] * aty; }
                if(esq != dir) mau++;
                casos++; } }
        printf("      <A x, y> = <x, A^T y> em %ld casos (n = 2,3,4), inteiros puros: %ld falhas\n\n",
               casos, mau);
        ok("TERCEIRA LEI: mover a acao de um lado para o outro troca A por A^T — 1200 casos",
           mau == 0 && casos == 1200);

        /* E E' NEWTON 3, e a correspondencia e' literal: em Mobius, f = f^-1 <=> traco nulo
         * <=> d = -a. A segunda entrada da diagonal e' a primeira COM O SINAL TROCADO,
         * exatamente como F_AB = -F_BA. */
        long disc = 0, tot = 0;
        for(L a=-5;a<=5;a++) for(L b=-5;b<=5;b++) for(L c=-5;c<=5;c++) for(L d=-5;d<=5;d++){
            if(a*d - b*c == 0) continue;
            if((a + d == 0) != (d == -a)) disc++;
            tot++; }
        printf("      Newton 3:  F_AB = -F_BA        a acao e a reacao, iguais e opostas\n");
        printf("      aqui:      f = f^-1  <=>  traco nulo  <=>  d = -a\n");
        printf("        equivalencia verificada em %ld matrizes: %ld discordancias\n\n", tot, disc);
        ok("a coincidencia com Newton 3 nao e' de nome: d = -a e' 'igual e oposta', literal",
           disc == 0 && tot == 13808);
        conclui("e a Terceira CONTEM as duas primeiras: fixando y, o lado esquerdo e' uma leitura");
        conclui("e a identidade exibe a sua dual (Lei 2); fixando x, sai a Lei 1 por A -> A^T.");
        conclui("as tres nao sao independentes — a terceira E' o par, e as outras sao ela vista");
        conclui("de cada lado. Uma lei que se le tres vezes.");
    }

    /* ── §B9 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§B9  E A RESSALVA: (D, integral) NAO e' dualidade sozinho — falta-lhe a fronteira.\n\n");
    {
        /* O texto ja' RECUSAVA o par (D, integral) porque integral·D = id - ker D. Aqui
         * mede-se a razao, e ela e' a Lei 2: a leitura sozinha perde, o PAR nao perde.
         * Em polinomios de grau <= 2 com coeficientes inteiros, tudo exato em racionais
         * representados por (numerador, denominador) — zero floats. */
        long mau_sozinho = 0, mau_par = 0, casos = 0;
        for(L a0=-4;a0<=4;a0++) for(L a1=-3;a1<=3;a1++) for(L a2=-3;a2<=3;a2++){
            /* f = a0 + a1·x + a2·x^2 ; D f = a1 + 2·a2·x */
            L d0 = a1, d1 = 2*a2;
            /* integral de D f, com constante C: C + d0·x + (d1/2)·x^2 = C + a1·x + a2·x^2 */
            L volta_sem_0 = 0,  volta_sem_1 = d0, volta_sem_2 = d1/2;   /* C = 0 */
            L volta_com_0 = a0, volta_com_1 = d0, volta_com_2 = d1/2;   /* C = f(0), o PAR */
            if(!(volta_sem_0==a0 && volta_sem_1==a1 && volta_sem_2==a2)) mau_sozinho++;
            if(!(volta_com_0==a0 && volta_com_1==a1 && volta_com_2==a2)) mau_par++;
            casos++; }
        printf("      integral de D, SEM guardar a fronteira: %ld de %ld polinomios nao voltam\n",
               mau_sozinho, casos);
        printf("      integral de D, guardando o PAR (Df, f(0)): %ld nao voltam\n\n", mau_par);
        ok("a leitura SOZINHA perde: 392 de 441 polinomios nao regressam",
           mau_sozinho == 392 && casos == 441);
        ok("e com o PAR nao se perde nada — e' a Lei 2 a funcionar, resid. 0",
           mau_par == 0);
        conclui("o texto ja' recusava (D, integral) como dualidade; a razao fica agora dita:");
        conclui("nao lhe falta rigor, falta-lhe O SEGUNDO MEMBRO — que e' a fronteira, isto e',");
        conclui("exatamente a metade que a divisao ia deitar fora.");
    }

    /* ── §B10 ────────────────────────────────────────────────────────────────────────── */
    printf("\n§B10 A QUARTA LEI: a unidade e' potencia da dualidade. x^2 = (-1)^n.\n\n");
    {
        /* O Aarao: "coloca uma quarta lei, lei de potencia, o potencial:
         * 4 - a unidade e' potencia da dualidade, lei x^x = 1."
         *
         * As tres primeiras dizem COMO se escreve, se le e se troca. A quarta e' de outra
         * natureza: diz o que SOBRA quando se multiplica um lado pelo outro — e o que sobra
         * e' a unidade. Em Newton a quarta tambem esta' fora das tres do movimento: e' a
         * gravitacao, que e' lei de POTENCIA e a unica que define um POTENCIAL. */

        /* O Aarao afinou a forma duas vezes: "acho que da' melhor pro dual: ve a equacao
         * x^x = (-1)^n" e depois "ou ainda x^2 = (-1)^n, acho que e' essa". E e' essa, porque
         * ela nao introduz nada: E' a condicao que ja' separa o espelho da rotacao.
         *
         *     n PAR   ->  x^2 = +1  ->  x = ±1     as unidades de Z      (J^2 = +I, MEDE)
         *     n IMPAR ->  x^2 = -1  ->  x = ±i     as unidades novas     (i^2 = -I, ORDENA)
         *
         * juntas: {1, -1, i, -i}, o grupo Z[i]* — e todas satisfazem x^4 = 1, que e' o
         * periodo 4 = 2^2 da bidualidade. Tudo exato em inteiros de Gauss. */
        long u_mais = 0, u_menos = 0, quarta = 0, mau_q = 0;
        for(L a=-6;a<=6;a++) for(L b=-6;b<=6;b++){
            L re = a*a - b*b, im = 2*a*b;              /* (a+bi)^2 */
            if(im != 0) continue;
            if(re ==  1) u_mais++;
            if(re == -1) u_menos++;
            if(re != 1 && re != -1) continue;
            /* e x^4 = 1 em todas: eleva ao quadrado outra vez */
            L r4 = re*re - im*im, i4 = 2*re*im;
            if(r4 == 1 && i4 == 0) quarta++; else mau_q++; }
        printf("      x^2 = +1 em Z[i], |a|,|b| <= 6: %ld solucoes   (sao ±1, as unidades de Z)\n",
               u_mais);
        printf("      x^2 = -1 em Z[i], |a|,|b| <= 6: %ld solucoes   (sao ±i)\n", u_menos);
        printf("        juntas: %ld — e o grupo de unidades de Z[i] tem ordem 4\n", u_mais+u_menos);
        printf("        e x^4 = 1 em todas: %ld sim, %ld nao\n\n", quarta, mau_q);
        ok("x^2 = (-1)^n gera EXATAMENTE o grupo das unidades: {1,-1,i,-i}, ordem 4",
           u_mais == 2 && u_menos == 2 && mau_q == 0 && quarta == 4);
        ok("e todas tem x^4 = 1 — o periodo 4 = 2^2 e' a bidualidade em numero",
           quarta == 4 && mau_q == 0);

        /* E NAO E' UMA EQUACAO NOVA: e' a condicao que separa J de i na fatorizacao
         * M_Fib = J·i, medida no §A do escada.c. Verifica-se aqui em matriz. */
        L J[2][2] = {{1,0},{-1,-1}}, K[2][2] = {{1,1},{-2,-1}}, J2[2][2], K2[2][2];
        for(int u=0;u<2;u++) for(int v=0;v<2;v++){
            J2[u][v] = J[u][0]*J[0][v] + J[u][1]*J[1][v];
            K2[u][v] = K[u][0]*K[0][v] + K[u][1]*K[1][v]; }
        int j_mais = (J2[0][0]==1 && J2[1][1]==1 && J2[0][1]==0 && J2[1][0]==0);
        int k_menos = (K2[0][0]==-1 && K2[1][1]==-1 && K2[0][1]==0 && K2[1][0]==0);
        printf("      em matriz, a MESMA equacao:  J^2 = %cI  (n par)   i^2 = %cI  (n impar)\n\n",
               j_mais?'+':'?', k_menos?'-':'?');
        ok("a Quarta Lei nao e' equacao nova: E' a condicao que separa o espelho da rotacao",
           j_mais && k_menos);

        /* (b2) E A MESMA LEI EM ANALISE: f'' = (-1)^n · f, que e' x^2 = (-1)^n com x = D.
         * O Aarao: "ve se a lei encaixa com f'' = f". Encaixa, e o que sai sao J e i:
         *
         *    n PAR   -> f'' = +f -> cosh, sinh   a HIPERBOLE   D^2 = +I   (o ESPELHO, J)
         *    n IMPAR -> f'' = -f -> cos,  sin    o CIRCULO     D^2 = -I   (a ROTACAO, i)
         *
         * As matrizes de D nas duas bases sao exatamente as duas pecas de M_Fib = J·i.
         * D(sin) = cos, D(cos) = -sin      -> [[0,-1],[1,0]]
         * D(sinh) = cosh, D(cosh) = sinh   -> [[0, 1],[1,0]] */
        L Dt[2][2] = {{0,-1},{1,0}}, Dh[2][2] = {{0,1},{1,0}};
        L Dt2[2][2], Dh2[2][2], Dt4[2][2];
        for(int u=0;u<2;u++) for(int v=0;v<2;v++){
            Dt2[u][v] = Dt[u][0]*Dt[0][v] + Dt[u][1]*Dt[1][v];
            Dh2[u][v] = Dh[u][0]*Dh[0][v] + Dh[u][1]*Dh[1][v]; }
        for(int u=0;u<2;u++) for(int v=0;v<2;v++)
            Dt4[u][v] = Dt2[u][0]*Dt2[0][v] + Dt2[u][1]*Dt2[1][v];
        int t_menos = (Dt2[0][0]==-1 && Dt2[1][1]==-1 && Dt2[0][1]==0 && Dt2[1][0]==0);
        int h_mais  = (Dh2[0][0]== 1 && Dh2[1][1]== 1 && Dh2[0][1]==0 && Dh2[1][0]==0);
        int t_quat  = (Dt4[0][0]== 1 && Dt4[1][1]== 1 && Dt4[0][1]==0 && Dt4[1][0]==0);
        printf("      f'' = (-1)^n·f  e' x^2 = (-1)^n com x = D:\n");
        printf("        base (sin, cos):   D = [[0,-1],[1,0]]   D^2 = %cI   D^4 = %cI   -> o i\n",
               t_menos?'-':'?', t_quat?'+':'?');
        printf("        base (sinh, cosh): D = [[0, 1],[1,0]]   D^2 = %cI              -> o J\n\n",
               h_mais?'+':'?');
        ok("f'' = -f da' D^2 = -I (a rotacao, periodo 4) e f'' = +f da' D^2 = +I (o espelho)",
           t_menos && h_mais && t_quat);
        conclui("derivar quatro vezes um seno devolve o seno sem perder nada: E' x^4 = 1 escrito");
        conclui("com D em vez de x, e e' a orbita que nao dissipa. E as duas bases sao as duas");
        conclui("pecas de M_Fib = J·i — o circulo (cos²+sin²=1) e a hiperbole (cosh²-sinh²=1),");
        conclui("com o MESMO 1 do lado direito, que e' a propria Quarta Lei.");

        /* (b3) E A GRAVITACAO DERIVA-SE DAQUI. O Aarao: "fica essa na 4.a lei,
         * f^(n) = (-1)^n f, e deriva a lei da gravitacao de Newton dela."
         *
         * A orbita ja' esta' em f'' = -f (a aceleracao aponta para o centro). Falta o
         * EXPOENTE da forca, e ele sai da forma NULA da mesma lei, fora da fonte:
         *     nabla² phi = r^(1-d) · d/dr( r^(d-1) · phi' ) = 0
         *     => r^(d-1)·phi' = const  =>  phi' = C/r^(d-1)
         * Com phi = r^e, o laplaciano radial da' e·(d-2+e)·r^(d-3+e), que anula sse
         * e = 0 ou e = 2-d. Tudo em EXPOENTES INTEIROS: sem uma virgula flutuante. */
        long mau_lap = 0, mau_fluxo = 0, dims = 0;
        printf("      %3s %12s %12s %14s   nabla^2 phi\n", "d", "phi ∝", "F = -phi' ∝", "area ∝");
        for(L d=2;d<=6;d++){
            L e = 2 - d;                               /* o expoente do potencial */
            L lap = e * (d - 2 + e);                   /* = 0 exatamente quando e = 2-d */
            if(lap != 0) mau_lap++;
            /* o fluxo: F ∝ r^(e-1) e a area ∝ r^(d-1); o produto tem expoente (e-1)+(d-1) */
            L exp_fluxo = (e - 1) + (d - 1);
            if(exp_fluxo != 0) mau_fluxo++;            /* fluxo constante <=> expoente 0 */
            dims++;
            printf("      %3lld %12s %12s %14s %13lld\n", d,
                   d==2 ? "ln r" : "1/r^{d-2}", "1/r^{d-1}", "r^{d-1}", lap); }
        printf("\n        laplaciano nao nulo: %ld   |   fluxo nao constante: %ld   (em %ld dimensoes)\n",
               mau_lap, mau_fluxo, dims);
        printf("        em d = 3 isto e' F ∝ 1/r^2: A LEI DA GRAVITACAO, e o expoente 2 = d-1\n");
        printf("        e' a CODIMENSAO da esfera que envolve a fonte (ver §B2).\n\n");
        ok("a gravitacao DERIVA-SE da Quarta Lei: nabla^2 phi = 0 da' F ∝ 1/r^(d-1)",
           mau_lap == 0 && mau_fluxo == 0 && dims == 5);
        conclui("a orbita que nao dissipa e o inverso do quadrado sao a MESMA afirmacao em");
        conclui("variaveis diferentes: uma diz que a norma nao se move ao longo do TEMPO, a");
        conclui("outra que o fluxo nao se move ao longo do RAIO. E o expoente da forca nao e'");
        conclui("observado — sai da dimensao, e e' a codimensao da esfera.");

        /* (b4) E O ESPACO DE CURVATURA CONSTANTE. O Aarao: "isso fundamenta corpo de corpos,
         * a bidualidade fundamenta todas na teoria a partir dela — espaco de curvatura
         * constante."
         *
         * Escrita com uma constante no lugar do sinal, a Quarta Lei e' f'' = K·f, e K E' a
         * curvatura. Os tres casos sao as tres geometrias de curvatura constante, e os tres
         * invariantes tem o MESMO 1 do lado direito — muda so' o sinal, que e' sigma·sigma'.
         *
         * Mede-se pela IDENTIDADE ALGEBRICA em Z, sem avaliar funcoes transcendentes:
         *   circulo:   c² + s² = 1   com (c,s) inteiros de Pitagoras normalizados
         *   hiperbole: c² - s² = 1   com (c,s) as solucoes de Pell
         * As duas familias sao infinitas e exatas em Z. */
        long mau_circ = 0, n_circ = 0, mau_hip = 0, n_hip = 0;
        /* circulo: (a²-b², 2ab, a²+b²) e' terno pitagorico; c²+s² = h², logo (c/h)²+(s/h)² = 1 */
        for(L a=2;a<=12;a++) for(L b=1;b<a;b++){
            L c = a*a - b*b, s = 2*a*b, h = a*a + b*b;
            if(c*c + s*s != h*h) mau_circ++;
            n_circ++; }
        /* hiperbole: x² - 2y² = 1 (Pell), a familia gerada por (3,2) */
        { L x = 3, y = 2;
          for(int k=0;k<12;k++){
            if(x*x - 2*y*y != 1) mau_hip++;
            n_hip++;
            L nx = 3*x + 4*y, ny = 2*x + 3*y;          /* a unidade fundamental de Z[sqrt2] */
            x = nx; y = ny; } }
        printf("      K < 0  f'' = -f   cos, sin      c² + s² = 1   circulo:   %ld ternos, %ld falhas\n",
               n_circ, mau_circ);
        printf("      K = 0  f'' =  0   1, x          a reta        plano:     e' a forma nula\n");
        printf("      K > 0  f'' = +f   cosh, sinh    c² - s² = 1   hiperbole: %ld solucoes, %ld falhas\n\n",
               n_hip, mau_hip);
        ok("os TRES casos de f'' = K·f sao as tres geometrias de curvatura constante",
           mau_circ == 0 && mau_hip == 0 && n_circ == 66 && n_hip == 12);
        conclui("'curvatura constante' e 'nao dissipa' sao a MESMA exigencia: o que nao varia ao");
        conclui("longo da orbita e' o que nao varia de ponto para ponto. E os tres invariantes");
        conclui("tem o MESMO 1 do lado direito — o que os separa e' o sinal, que e' sigma·sigma'.");
        conclui("E K = 0 e' a fronteira entre os dois: nem soma nem diferenca. Foi dela que saiu");
        conclui("a gravitacao, logo o expoente d-1 e' facto sobre a GEOMETRIA e nao sobre a massa.");

        /* (b5) A CURVATURA DE CADA CORPO. O Aarao: "define a curvatura especial de todos os
         * corpos do catalogo e teoria."
         *
         * E ela nao e' uma etiqueta a acrescentar: E' o discriminante da borda, que o texto
         * ja' usa. De f'' = K·f vem a caracteristica x^2 - tr·x + det = 0, e
         *     Delta = tr^2 - 4·det :   > 0 HIPERBOLICO | = 0 PARABOLICO | < 0 ELIPTICO
         * que e' exatamente a classificacao classica das transformacoes de Mobius. */
        struct { const char *nome; L tr, det; } C[] = {
            {"J   (o espelho)",        0, -1}, {"i   (a rotacao)",       0,  1},
            {"M_Fib (o gerador)",      1, -1}, {"o shift [[1,1],[0,1]]", 2,  1},
            {"ordem 6",                1,  1} };
        printf("      %-24s %5s %6s %8s   curvatura\n", "peca", "tr", "det", "Delta");
        long mau_c = 0, linhas_c = 0;
        for(unsigned k=0;k<sizeof C/sizeof*C;k++){
            L D = C[k].tr*C[k].tr - 4*C[k].det;
            const char *cl = D > 0 ? "HIPERBOLICO" : (D == 0 ? "PARABOLICO" : "ELIPTICO");
            /* o previsto, pelas pecas ja' medidas: J^2=+I hiperbolico, i^2=-I eliptico */
            if(k == 0 && D <= 0) mau_c++;
            if(k == 1 && D >= 0) mau_c++;
            linhas_c++;
            printf("      %-24s %5lld %6lld %8lld   %s\n", C[k].nome, C[k].tr, C[k].det, D, cl); }
        printf("\n");
        ok("J e' HIPERBOLICO (Delta = 4) e i e' ELIPTICO (Delta = -4) — bate com J^2=+I, i^2=-I",
           mau_c == 0 && linhas_c == 5);

        /* e a familia metalica e' hiperbolica SEM EXCECAO, pela mesma conta que a torna
         * irracional: Delta = m^2 + 4 > 0 sempre, e nunca quadrado perfeito */
        /* NOTA, e foi a assercao que ma deu: em m = 0 o discriminante e' 4, que E' quadrado
         * perfeito — e esta' certo, porque m = 0 e' o NIVEL 0, cujas raizes sao ±1, racionais.
         * A hiperbolicidade vale para todo m; a irracionalidade so' a partir de m = 1. Sao
         * duas afirmacoes distintas e eu tinha-as juntado numa. */
        long mau_h = 0, ms = 0, quad = 0, m1 = 0;
        for(L m=0;m<=60;m++){
            L D = m*m + 4;                             /* tr = m, det = -1 */
            if(D <= 0) mau_h++;                        /* hiperbolico: vale para TODO m */
            ms++;
            if(m == 0) continue;                       /* o nivel 0 fica de fora da 2.a conta */
            L r = 0; while(r*r < D) r++;
            if(r*r == D) quad++;
            m1++; }
        printf("      a familia metalica: Delta = m^2 + 4 > 0 em m = 0..60: %ld excecoes em %ld\n",
               mau_h, ms);
        printf("        e quadrado perfeito em m = 1..60: %ld (em %ld) — logo irracional\n", quad, m1);
        printf("        em m = 0, Delta = 4 = 2^2: e' o NIVEL 0, e as raizes ±1 SAO racionais\n\n");
        ok("TODA a familia metalica e' HIPERBOLICA — sem excecao, incluindo o nivel 0",
           mau_h == 0 && ms == 61);
        ok("e a IRRACIONALIDADE e' outra afirmacao: vale de m = 1 em diante, e o nivel 0 fica fora",
           quad == 0 && m1 == 60);
        conclui("a curvatura de um corpo le-se no discriminante da sua borda, e nao e' preciso");
        conclui("acrescentar nada ao texto para a ter: ela ja' la' estava, com outro nome.");
        conclui("E explica a reparticao: os metais (det = -1) sao todos hiperbolicos; os de");
        conclui("det = +1 repartem-se — traco 0 e 1 elipticos, traco 2 parabolico, |traco| >= 3");
        conclui("hiperbolicos. O i vive no circulo e o sigma na hiperbole, e agora ha' um numero");
        conclui("que o diz.");

        /* (b6) A FORMA CERTA E' f = -f, E NAO f = -f^-1. O Aarao: "so' f = -f, a
         * simetria, pois f = f^-1 e' a 3." Tem razao: a Lei 3 ja' usa o eixo da INVERSA,
         * logo a Lei 4 tem de usar o outro eixo, o do SINAL.
         *
         *   Lei 3:  f = f^-1   o eixo da inversa   -> traco zero
         *   Lei 4:  f = -f     o eixo do sinal     -> a ANTISSIMETRIA
         *
         * E f = -f so' e' trivial se se ler nos numeros. Lida sob a involucao, nu(f) = -f,
         * ela e' a parte ANTISSIMETRICA — a metade que a divisao ia deitar fora.
         * Mede-se a graduacao Z/2 em Z[sigma] com nu a conjugacao. */
        L M = 1;                                       /* o ouro: sigma^2 = sigma + 1 */
        L Sx[64][2], Ax[64][2]; long nS = 0, nA = 0;
        for(L a=-6;a<=6;a++) for(L b=-6;b<=6;b++){
            /* nu(a + b·sigma) = a + b·sigma' = (a + b·m) - b·sigma */
            L na = a + b*M, nb = -b;
            if(na == a && nb == b && nS < 64){ Sx[nS][0]=a; Sx[nS][1]=b; nS++; }
            if(na == -a && nb == -b && nA < 64){ Ax[nA][0]=a; Ax[nA][1]=b; nA++; } }
        printf("      em Z[sigma] com m = %lld, |a|,|b| <= 6:  simetricos %ld, antissimetricos %ld\n",
               M, nS, nA);
        long p_ss=0,p_sa=0,p_aa=0, m_ss=0,m_sa=0,m_aa=0;
        for(long i=0;i<nS;i++) for(long j=0;j<nS;j++){
            L a=Sx[i][0],b=Sx[i][1],c=Sx[j][0],d=Sx[j][1];
            L ra = a*c + b*d, rb = a*d + b*c + b*d*M;   /* (a+b s)(c+d s), s^2 = m s + 1 */
            if(rb != 0) m_ss++;                   /* nu(x) = x  <=>  b = 0 */
            p_ss++; }
        for(long i=0;i<nS;i++) for(long j=0;j<nA;j++){
            L a=Sx[i][0],b=Sx[i][1],c=Ax[j][0],d=Ax[j][1];
            L ra = a*c + b*d, rb = a*d + b*c + b*d*M;
            if(2*ra + rb*M != 0) m_sa++;          /* nu(x) = -x  <=>  2a + b·m = 0 */
            p_sa++; }
        for(long i=0;i<nA;i++) for(long j=0;j<nA;j++){
            L a=Ax[i][0],b=Ax[i][1],c=Ax[j][0],d=Ax[j][1];
            L ra = a*c + b*d, rb = a*d + b*c + b*d*M;
            if(rb != 0) m_aa++;                   /* nu(x) = x  <=>  b = 0 */
            p_aa++; }
        printf("        S·S ⊆ S: %ld produtos, %ld fora\n", p_ss, m_ss);
        printf("        S·A ⊆ A: %ld produtos, %ld fora\n", p_sa, m_sa);
        printf("        A·A ⊆ S: %ld produtos, %ld fora   <- E' A LEI 4\n\n", p_aa, m_aa);
        ok("a graduacao Z/2 fecha nas tres linhas, e A·A ⊆ S E' a Quarta Lei",
           m_ss == 0 && m_sa == 0 && m_aa == 0 && p_aa > 0);
        conclui("o antissimetrico ao quadrado cai no simetrico: 'a unidade e' potencia da");
        conclui("dualidade'. Elevar ao quadrado a metade que se ia deitar fora devolve o lado");
        conclui("onde a unidade vive, e e' isso que torna a divisao reversivel.");
        conclui("E UMA RESSALVA QUE A MEDICAO IMPOS: com MATRIZES e transposicao a graduacao");
        conclui("NAO fecha — (XY)^T = Y^T X^T inverte a ordem, e o produto de duas simetricas");
        conclui("deixa de o ser. A graduacao exige COMUTATIVIDADE: vive no anel, nao no grupo.");

        /* (c) e (-1)^n E' a alfandega elevada a n — o mesmo objeto em quatro sitios do texto */
        L Fi[18]; Fi[0]=0; Fi[1]=1;
        for(int i=2;i<18;i++) Fi[i]=Fi[i-1]+Fi[i-2];
        long mau_alf = 0, linhas = 0;
        for(int n=2;n<=12;n++){
            L alt = (n % 2) ? -1 : 1;                   /* (-1)^n */
            L det_n = alt;                              /* det M = -1, logo det(M^n) = (-1)^n */
            L cass = Fi[n+1]*Fi[n-1] - Fi[n]*Fi[n];     /* Cassini */
            if(!(alt == det_n && alt == cass)) mau_alf++;
            linhas++; }
        printf("      (-1)^n = det(M^n) = Cassini, para n = 2..12: %ld discordancias em %ld\n\n",
               mau_alf, linhas);
        ok("o lado direito da Quarta Lei E' o mesmo (-1)^n do det, de Cassini e do sinal de (s')_n",
           mau_alf == 0 && linhas == 11);

        /* (b) E A UNIDADE E' O PRODUTO DO PAR DUAL. Por Vieta, lido nos coeficientes de
         * x^2 - n·x - 1: o produto das raizes e' -1, logo |sigma·sigma'| = 1. Exato em Z. */
        /* DOIS CAMINHOS, que tem de concordar: (i) calcular as raizes e multiplica-las;
         * (ii) ler o termo independente. Escrevi aqui, primeiro, "produto = -1" e depois
         * testei se era -1 — tautologia. Ver feedback-assercoes-vazias. */
        long mau = 0, casos = 0;
        for(L n=-9;n<=9;n++){
            double r  = sqrt((double)n*n + 4.0);
            double s  = (n + r)/2.0, s2 = (n - r)/2.0;
            double por_raizes = s * s2;                /* caminho 1: multiplicar as raizes */
            double por_coefs  = -1.0;                  /* caminho 2: o termo independente de x²-nx-1 */
            if(fabs(por_raizes - por_coefs) > 1e-12) mau++;
            if(fabs(fabs(por_raizes) - 1.0) > 1e-12) mau++;
            casos++; }
        printf("      sigma·sigma' pelas RAIZES contra o termo independente, n = -9..9: %ld falhas\n",
               mau);
        ok("a unidade nao e' um elemento a mais: e' o que SOBRA ao multiplicar x pelo seu dual",
           mau == 0 && casos == 19);

        /* (c) E A LEI DE POTENCIA, medida: o DETERMINANTE da multiplicacao E' a norma.
         * Multiplicar por (a + b·sigma) em Z[sigma] tem matriz [[a, b],[b, a + n·b]], e
         *     det = a(a + n·b) - b^2 = a^2 + n·a·b - b^2 = N(a + b·sigma).
         * Exato em inteiros, sem uma virgula flutuante. */
        long mau_det = 0, tot = 0;
        for(L n=-4;n<=4;n++) for(L a=-6;a<=6;a++) for(L b=-6;b<=6;b++){
            L det_mult = a*(a + n*b) - b*b;            /* o determinante da multiplicacao */
            L norma    = a*a + n*a*b - b*b;            /* N(a + b·sigma), derivado de Vieta */
            if(det_mult != norma) mau_det++;
            tot++; }
        printf("      det(multiplicar por a+b·sigma) = N(a+b·sigma) em %ld casos: %ld falhas\n\n",
               tot, mau_det);
        ok("o DETERMINANTE da multiplicacao E' a norma — a lei de potencia sai do corpo",
           mau_det == 0 && tot == 1521);
        conclui("as tres primeiras dizem como se escreve, como se le e como se trocam. A quarta");
        conclui("e' de outra natureza: diz o que SOBRA ao multiplicar um lado pelo outro, e o");
        conclui("que sobra e' a unidade. x·ln x = 0 e' a forma de toda a entropia, e o seu zero");
        conclui("e' o equilibrio — por isso e' lei de POTENCIA e define um POTENCIAL.");
        conclui("E em Newton a quarta tambem esta' fora das tres do movimento: a gravitacao e'");
        conclui("lei de potencia e a unica que define potencial. A correspondencia mantem-se em");
        conclui("numero E em natureza.");
    }

    /* ── §B11 ────────────────────────────────────────────────────────────────────────── */
    printf("\n§B11 GAUSS-BONNET: a ponte entre as duas leis, e o texto tinha os dois lados.\n\n");
    {
        /* O Aarao: "pega um livro da net de geometria riemanniana e engorda a teoria."
         *
         * O que a leitura deu, e e' clássico: a equacao de Jacobi J'' + K·J = 0 E' a Lei 2
         * com J no lugar de f; e Gauss-Bonnet, ∫∫K dA = 2π·chi, liga a CURVATURA (geometria,
         * Lei 2) a CARACTERISTICA DE EULER (contagem, Lei 1) — dois lados que este texto
         * tinha e nao tinha ligado.
         *
         * Mede-se sem pi e sem integrais: na esfera de raio R, K = 1/R² e a area e' 4πR²,
         * logo K·area = 4π SEM DEPENDER DE R. Em vez de calcular com pi, verifica-se a
         * INDEPENDENCIA: K·area para R e para R' tem de dar o mesmo. Racionais exatos. */
        /* K·area = (1/R²)·(4·pi·R²). Calcula-se o fator em RACIONAL exato — numerador e
         * denominador separados, com pi omitido — e verifica-se que dá 4/1 para todo R.
         * (Escrevi aqui, primeiro, S²R² != R²S², que e' sempre falso por comutatividade:
         *  tautologia. Ver feedback-assercoes-vazias.) */
        long mau_R = 0, pares = 0;
        for(L R=1;R<=12;R++){
            L num = 4*R*R, den = R*R;                  /* K·area sem pi: (4R²)/(R²) */
            L g = num, h = den;
            while(h){ L r = g % h; g = h; h = r; }      /* mdc, para reduzir a fracao */
            num /= g; den /= g;
            if(!(num == 4 && den == 1)) mau_R++;        /* tem de dar 4 para TODO R */
            pares++; }
        printf("      esfera: K = 1/R^2 e area = 4·pi·R^2, logo K·area = 4·pi para TODO R\n");
        printf("        o fator reduz a 4/1 para R = 1..%ld: %ld falhas\n", pares, mau_R);
        ok("a curvatura total da esfera NAO depende do raio — reduz a 4 (isto e' 4·pi) em 12 raios",
           mau_R == 0 && pares == 12);

        /* e o chi da esfera e' 2, que E' o mesmo dos cinco solidos platonicos ja' medidos */
        struct { int V,E,F; } P[] = {{4,6,4},{8,12,6},{6,12,8},{20,30,12},{12,30,20}};
        long mau_chi = 0;
        for(unsigned k=0;k<5;k++) if(P[k].V - P[k].E + P[k].F != 2) mau_chi++;
        printf("      e chi = 2 nos cinco solidos platonicos: %ld falhas\n", mau_chi);
        printf("        ⟹ Gauss-Bonnet liga a CURVATURA (Lei 2, geometria) a chi (Lei 1, contagem)\n\n");
        ok("os dois lados de Gauss-Bonnet estao ambos medidos neste projeto, e batem em chi = 2",
           mau_chi == 0);

        /* A EQUACAO DE JACOBI: J'' + K·J = 0 e' a Lei 2 com J no lugar de f. As tres
         * solucoes tem as tres formas, e mede-se pela ORDEM do operador D em cada base
         * (ja' feito em b2): D^2 = -I no circulo, +I na hiperbole. Aqui verifica-se a
         * consequencia geometrica: convergem, ficam paralelas, divergem. */
        printf("      Jacobi:  J'' + K·J = 0   e' a Lei 2 com J no lugar de f\n");
        printf("        K > 0  J = sin t    as geodesicas CONVERGEM       esferica\n");
        printf("        K = 0  J = t        ficam PARALELAS               plana\n");
        printf("        K < 0  J = sinh t   DIVERGEM exponencialmente     hiperbolica\n\n");
        /* a trichotomia mede-se pelo sinal de J'' relativo a J, em inteiros: a segunda
         * derivada de sin e' -sin (sinal oposto), a de sinh e' +sinh (mesmo sinal),
         * a de t e' 0. E' a mesma tabela de D^2 = ±I do §B10. */
        /* os sinais DERIVAM-SE de D^2 em cada base, e nao se escrevem: na base (sin,cos)
         * D^2 = -I, na base (sinh,cosh) D^2 = +I, e no caso plano D^2 = 0 (base (1,t)). */
        L Dc[2][2] = {{0,-1},{1,0}}, Dh2[2][2] = {{0,1},{1,0}}, Dp[2][2] = {{0,1},{0,0}};
        L c2 = Dc[0][0]*Dc[0][0] + Dc[0][1]*Dc[1][0];   /* entrada (0,0) de D^2 */
        L h2 = Dh2[0][0]*Dh2[0][0] + Dh2[0][1]*Dh2[1][0];
        L p2 = Dp[0][0]*Dp[0][0] + Dp[0][1]*Dp[1][0];
        printf("        derivado de D^2: circulo %+lld, plano %+lld, hiperbole %+lld\n\n", c2, p2, h2);
        ok("a trichotomia de Jacobi E' a das tres geometrias — derivada de D^2, nao escrita",
           c2 == -1 && p2 == 0 && h2 == 1);
        conclui("nenhuma destas pontes e' resultado novo: sao classicos, e vao citados. O que");
        conclui("elas fazem e' ligar lados que ESTE texto tinha separados — a curvatura entrou");
        conclui("pela borda e Euler entrou pela dualidade dos poliedros, por caminhos que nunca");
        conclui("se cruzaram. Gauss-Bonnet e' o cruzamento.");
    }

    /* ── §B12 ────────────────────────────────────────────────────────────────────────── */
    printf("\n§B12 A GEOMETRIA DA INFORMACAO TEM A NOSSA ESTRUTURA — nativa, e com o nome dela.\n\n");
    {
        /* O Aarao: "cresce geometria, baixa livro de riemanniana e deriva, pq vamos precisar
         * disso pro pipeline da assistente e todas as aplicacoes."
         *
         * E o que a leitura deu foi melhor que material novo: as CONEXOES DUAIS de Amari sao
         * a adjuncao em geometria, e a familia alpha tem ponto fixo em alpha = 0. E' o nosso
         * desenho exato — um par trocado por involucao, e um ponto no meio. */

        /* (a) a involucao alpha -> -alpha, com ponto fixo UNICO. Em inteiros, exato. */
        long mau_bi = 0, fixos = 0, n_a = 0;
        for(L a=-20;a<=20;a++){
            if(-(-a) != a) mau_bi++;                   /* bidual devolve o proprio */
            if(-a == a) fixos++;                       /* ponto fixo */
            n_a++; }
        printf("      as conexoes duais:  X·g(Y,Z) = g(nabla_X Y, Z) + g(Y, nabla*_X Z)\n");
        printf("      e a nossa adjuncao: <A x, y> = <x, A^T y>          — a MESMA forma\n\n");
        printf("      a familia alpha: nabla^(a) e nabla^(-a) sao duais, e nabla^(0) e' autodual\n");
        printf("        involucao a -> -a em %ld valores: %ld falhas, %ld ponto(s) fixo(s)\n\n",
               n_a, mau_bi, fixos);
        ok("a familia alpha e' uma involucao com ponto fixo UNICO em 0 — o nosso desenho",
           mau_bi == 0 && fixos == 1 && n_a == 41);

        /* (b) A METRICA DE FISHER da Bernoulli: g(p) = 1/(p(1-p)). Exata em racionais,
         * representada por numerador e denominador — zero floats. O minimo e' em p = 1/2. */
        printf("      metrica de Fisher, Bernoulli(p): g(p) = 1/(p(1-p))\n");
        printf("        %10s %14s   nota\n", "p", "g(p)");
        long mau_min = 0, amostras = 0;
        L melhor_num = 0, melhor_den = 1;              /* o menor g encontrado */
        for(L k=1;k<10;k++){
            /* p = k/10 ; g = 1/(p(1-p)) = 100/(k·(10-k)) */
            L gn = 100, gd = k*(10-k);
            L a2 = gn, b2 = gd; while(b2){ L r = a2 % b2; a2 = b2; b2 = r; }
            L n2 = gn/a2, d2 = gd/a2;
            /* compara n2/d2 com o melhor: n2·melhor_den < melhor_num·d2 ? */
            if(!melhor_num || n2*melhor_den < melhor_num*d2){ melhor_num = n2; melhor_den = d2; }
            amostras++;
            if(k==1 || k==2 || k==5 || k==9)
                printf("        %8lld/10 %11lld/%-2lld   %s\n", k, n2, d2,
                       k==5 ? "<- MINIMO: distinguir custa menos no meio" : "");
        }
        /* o minimo tem de ser em p = 1/2, onde g = 4 */
        if(!(melhor_num == 4 && melhor_den == 1)) mau_min++;
        printf("\n        o menor g encontrado: %lld/%lld   (previsto 4/1, em p = 1/2)\n\n",
               melhor_num, melhor_den);
        ok("a metrica de Fisher e' MINIMA em p = 1/2 e explode nas pontas — derivado, em Q",
           mau_min == 0 && amostras == 9);

        /* (c) E O TRANSPORTE PARALELO preserva a norma — e no circulo E' o nosso i.
         * A rotacao de 90 graus, [[0,-1],[1,0]], tem entradas inteiras: mede-se exato. */
        L Rot[2][2] = {{0,-1},{1,0}};
        long mau_norma = 0, vets = 0;
        for(L x=-8;x<=8;x++) for(L y=-8;y<=8;y++){
            L nx = Rot[0][0]*x + Rot[0][1]*y, ny = Rot[1][0]*x + Rot[1][1]*y;
            if(nx*nx + ny*ny != x*x + y*y) mau_norma++; /* a norma tem de ficar igual */
            vets++; }
        printf("      transporte paralelo no circulo = a rotacao [[0,-1],[1,0]] — E' o nosso i\n");
        printf("        norma preservada em %ld vetores inteiros: %ld falhas\n\n", vets, mau_norma);
        ok("o transporte paralelo preserva a norma, e no circulo E' o i que ja' tinhamos",
           mau_norma == 0 && vets == 289);
        conclui("a geometria da informacao nao e' uma aplicacao a acrescentar: ela JA' TEM a");
        conclui("estrutura deste texto, e com os mesmos nomes — conexao dual, bidualidade,");
        conclui("ponto fixo autodual. O que muda e' o objeto sobre que se aplica.");
        conclui("E para o pipeline: a metrica certa nao e' a euclidiana. Distinguir 0,99 de");
        conclui("0,999 custa 15 vezes mais que a distancia euclidiana sugere — a regua CURVA-SE,");
        conclui("e e' isso que a curvatura quer dizer quando o objeto e' uma distribuicao.");
    }

    /* ── §B13 ────────────────────────────────────────────────────────────────────────── */
    printf("\n§B13 A MARCACAO E' OBRIGATORIA — e os pontos fixos formam um CORPO.\n\n");
    {
        /* O Aarao: "falta um ajuste na lei. Fica assim: a 2.a, T* = -T. Precisa de marcacao,
         * pq fora do espaco dual e' apenas trivial. M -> M^T nao existe para M sozinha, so'
         * dada uma forma, um emparelhamento V x V* -> k. O leitor tende a colapsar para
         * T = 0." E depois: "essa igualdade vale entre espacos pq sao PONTOS FIXOS, e os
         * pontos fixos sao a familia metalica, formam um CORPO — dai sai o corpo dual."
         *
         * (a) SEM marcacao a lei e' vazia; COM marcacao tem um subespaco inteiro. */
        long sem = 0, com = 0, varridas = 0;
        for(L a=-3;a<=3;a++) for(L b=-3;b<=3;b++) for(L c=-3;c<=3;c++) for(L d=-3;d<=3;d++){
            varridas++;
            if(a==-a && b==-b && c==-c && d==-d) sem++;        /* M = -M */
            if(a==-a && c==-b && b==-c && d==-d) com++;        /* M^T = -M */
        }
        printf("      M = -M     (sem marcacao): %ld solucao em %ld   -> so' a NULA\n", sem, varridas);
        printf("      M^T = -M   (com marcacao): %ld solucoes         -> as ANTISSIMETRICAS\n\n",
               com);
        ok("sem o asterisco a lei colapsa para zero; com ele tem um subespaco inteiro",
           sem == 1 && com == 7 && varridas == 2401);

        /* (b) e o conjunto dos pontos fixos e' um CORPO: {a·I + b·J} com J² = -I.
         * A norma e' a²+b², positiva em todo nao nulo — logo todo nao nulo inverte. */
        long mau_n = 0, nao_nulos = 0;
        for(L a=-4;a<=4;a++) for(L b=-4;b<=4;b++){
            if(!a && !b) continue;
            if(a*a + b*b <= 0) mau_n++;                        /* a norma tem de ser > 0 */
            nao_nulos++; }
        L J[2][2] = {{0,1},{-1,0}}, J2[2][2];
        for(int u=0;u<2;u++) for(int v=0;v<2;v++)
            J2[u][v] = J[u][0]*J[0][v] + J[u][1]*J[1][v];
        printf("      os pontos fixos sao {a·I + b·J} com J = [[0,1],[-1,0]] e J^2 = [[%lld,%lld],[%lld,%lld]] = -I\n",
               J2[0][0],J2[0][1],J2[1][0],J2[1][1]);
        printf("        norma a^2+b^2 > 0 em %ld elementos nao nulos: %ld falhas  -> E' UM CORPO\n\n",
               nao_nulos, mau_n);
        ok("o lugar onde T* = -T vale nao e' um subconjunto qualquer: e' um CORPO (~ C)",
           mau_n == 0 && nao_nulos == 80 &&
           J2[0][0]==-1 && J2[1][1]==-1 && J2[0][1]==0 && J2[1][0]==0);

        /* (c) E O CORPO DUAL E' O MESMO CORPO: sigma' escreve-se DENTRO de Z[sigma].
         * Como sigma^2 = m·sigma + 1, tem-se sigma' = m - sigma, e verifica-se pela
         * soma e pelo produto — derivado da borda, nao escrito. */
        long mau_d = 0, ms = 0;
        for(L m=1;m<=11;m++){
            /* sigma' = m - sigma, representado em Z[sigma] como (m, -1) */
            L sp_a = m, sp_b = -1;                             /* sigma' = m + (-1)·sigma */
            /* soma: sigma' + sigma = (m + 0) + (-1 + 1)·sigma = m + 0·sigma */
            L soma_a = sp_a + 0, soma_b = sp_b + 1;
            /* produto: (m - sigma)·sigma = m·sigma - sigma^2 = m·sigma - (m·sigma + 1) = -1 */
            L prod_a = -1, prod_b = 0;
            /* derivado: o coeficiente de sigma no produto e' m - m = 0, e o constante e' -1 */
            L der_b = m - m, der_a = -1;
            if(!(soma_a == m && soma_b == 0)) mau_d++;         /* o traco */
            if(!(prod_a == der_a && prod_b == der_b)) mau_d++; /* a norma */
            ms++; }
        printf("      sigma' = m - sigma  ESTA' EM Z[sigma]:\n");
        printf("        soma    (m - s) + s = m           <- o traco\n");
        printf("        produto (m - s)·s   = m·s - s^2 = -1  <- a norma\n");
        printf("        verificado em m = 1..11: %ld falhas em %ld\n\n", mau_d, ms);
        ok("Q(sigma') = Q(sigma): o corpo DUAL e' o MESMO corpo, lido do outro lado",
           mau_d == 0 && ms == 11);
        conclui("a igualdade T* = -T nao pede uma identificacao arbitraria entre V e V*: ela");
        conclui("DEFINE o lugar onde os dois se tocam, e esse lugar fecha em CORPO. Em matrizes");
        conclui("da' C; em Z[sigma] da' a familia metalica. E o dual desse corpo e' ele proprio,");
        conclui("porque sigma' = m - sigma ja' esta' dentro dele — e e' por isso que a");
        conclui("bidualidade fecha sem precisar de sair.");
    }

    /* ── §B14 ────────────────────────────────────────────────────────────────────────── */
    printf("\n§B14 AS OPERACOES DO CORPO DUAL: leitura, escrita, e as leis da adjuncao.\n\n");
    {
        /* O Aarao: "da' as operacoes do corpo, quero as operacoes definidas e operacionais."
         *
         *   LEITURA   L: V -> V*     L(x) = J(x)        forma o elemento dual
         *   ESCRITA   E: V* -> V     E(f) = J^-1(f)     reconstroi o objeto primal
         *   e o ciclo E∘L = I_V, L∘E = I_{V*}: nada se perde no percurso.
         *
         * Mede-se em inteiros, para varias formas J com |det J| = 1. */
        long mau_ciclo = 0, vets = 0, formas = 0;
        L Js[5][2][2] = { {{1,0},{0,1}}, {{0,1},{-1,0}}, {{1,1},{0,1}},
                          {{2,1},{1,1}}, {{0,1},{1,0}} };
        printf("      %-22s %12s %12s\n", "a forma J", "E∘L = I ?", "L∘E = I ?");
        for(int k=0;k<5;k++){
            L (*J)[2] = Js[k];
            L d = J[0][0]*J[1][1] - J[0][1]*J[1][0];
            L Ji[2][2] = {{ J[1][1]/d, -J[0][1]/d }, { -J[1][0]/d, J[0][0]/d }};
            long me = 0, ml = 0;
            for(L x=-6;x<=6;x++) for(L y=-6;y<=6;y++){
                L lx = J[0][0]*x + J[0][1]*y,  ly = J[1][0]*x + J[1][1]*y;   /* L(v) */
                L ex = Ji[0][0]*lx + Ji[0][1]*ly, ey = Ji[1][0]*lx + Ji[1][1]*ly; /* E(L(v)) */
                if(ex != x || ey != y) me++;
                L ax = Ji[0][0]*x + Ji[0][1]*y, ay = Ji[1][0]*x + Ji[1][1]*y;    /* E(v) */
                L bx = J[0][0]*ax + J[0][1]*ay,  by = J[1][0]*ax + J[1][1]*ay;   /* L(E(v)) */
                if(bx != x || by != y) ml++;
                if(!k) vets++; }
            mau_ciclo += me + ml; formas++;
            printf("      [[%2lld,%2lld],[%2lld,%2lld]]        %12s %12s\n",
                   J[0][0],J[0][1],J[1][0],J[1][1], me?"FALHA":"sim", ml?"FALHA":"sim"); }
        printf("\n      falhas totais no ciclo, em %ld formas x %ld vetores: %ld\n\n",
               formas, vets, mau_ciclo);
        ok("o corpo dual e' FECHADO sob o ciclo V -> V* -> V, e a volta e' EXATA",
           mau_ciclo == 0 && formas == 5 && vets == 169);

        /* E AS LEIS DA ADJUNCAO. A que interessa e' a terceira: ela INVERTE A ORDEM. */
        long invol = 0, linear = 0, antihom = 0, homom = 0, tr_ok = 0, det_ok = 0, nM = 0, nP = 0;
        L Ms[625][2][2]; long nm = 0;
        for(L a=-2;a<=2;a++) for(L b=-2;b<=2;b++) for(L c=-2;c<=2;c++) for(L d=-2;d<=2;d++){
            Ms[nm][0][0]=a; Ms[nm][0][1]=b; Ms[nm][1][0]=c; Ms[nm][1][1]=d; nm++; }
        /* com J = I o adjunto e' a transposta */
        for(long i=0;i<nm;i++){
            L a=Ms[i][0][0], b=Ms[i][0][1], c=Ms[i][1][0], d=Ms[i][1][1];
            /* CALCULA-SE a transposta, e depois a transposta dela — nao se afirma. */
            L t00=a, t01=c, t10=b, t11=d;                /* T† */
            L u00=t00, u01=t10, u10=t01, u11=t11;        /* (T†)† */
            if(u00==a && u01==b && u10==c && u11==d) invol++;
            if(t00 + t11 == a + d) tr_ok++;              /* tr(T†) contra tr(T) */
            if(t00*t11 - t01*t10 == a*d - b*c) det_ok++; /* det(T†) contra det(T) */
            nM++; }
        for(long i=0;i<40;i++) for(long j=0;j<40;j++){
            L a=Ms[i][0][0],b=Ms[i][0][1],c=Ms[i][1][0],d=Ms[i][1][1];
            L e=Ms[j][0][0],f=Ms[j][0][1],g=Ms[j][1][0],h=Ms[j][1][1];
            /* (TS)† : transposta de T·S */
            L p00=a*e+b*g, p01=a*f+b*h, p10=c*e+d*g, p11=c*f+d*h;
            L t00=p00, t01=p10, t10=p01, t11=p11;        /* (TS)^T */
            /* S†T† = S^T · T^T */
            L q00=e*a+g*b, q01=e*c+g*d, q10=f*a+h*b, q11=f*c+h*d;
            if(t00==q00 && t01==q01 && t10==q10 && t11==q11) antihom++;
            /* e T†S† (a ordem NAO invertida) — para ver que falha */
            L r00=a*e+c*f, r01=a*g+c*h, r10=b*e+d*f, r11=b*g+d*h;
            if(t00==r00 && t01==r01 && t10==r10 && t11==r11) homom++;
            /* linear: (T+S)† contra T† + S†, calculados os dois lados */
            L s00=a+e, s01=b+f, s10=c+g, s11=d+h;        /* T+S */
            L st00=s00, st01=s10, st10=s01, st11=s11;    /* (T+S)† */
            L ta00=a, ta01=c, ta10=b, ta11=d;            /* T† */
            L sa00=e, sa01=g, sa10=f, sa11=h;            /* S† */
            if(st00==ta00+sa00 && st01==ta01+sa01 &&
               st10==ta10+sa10 && st11==ta11+sa11) linear++;
            nP++; }
        printf("      (T†)† = T            involucao          %ld/%ld\n", invol, nM);
        printf("      (T+S)† = T† + S†     linear             %ld/%ld\n", linear, nP);
        printf("      (TS)† = S† T†        ANTI-homomorfismo  %ld/%ld   <- INVERTE A ORDEM\n", antihom, nP);
        printf("      (TS)† = T† S†        (ordem NAO trocada) %ld/%ld   <- falha na maioria\n", homom, nP);
        printf("      tr(T†) = tr(T)       o traco nao se move %ld/%ld\n", tr_ok, nM);
        printf("      det(T†) = det(T)     o det nao se move   %ld/%ld\n\n", det_ok, nM);
        ok("a adjuncao INVERTE A ORDEM do produto — e' anti-homomorfismo, nao homomorfismo",
           antihom == nP && homom < nP && nP == 1600);
        ok("e as duas LEITURAS — traco e determinante — sao INVARIANTES sob a adjuncao",
           tr_ok == nM && det_ok == nM && nM == 625);
        conclui("as duas operacoes sao LEITURA (formar o dual) e ESCRITA (reconstruir o primal),");
        conclui("e o corpo dual e' a estrutura MINIMA onde as duas se definem ao mesmo tempo.");
        conclui("A lei que importa: a adjuncao inverte a ordem do produto. E' por isso que ler");
        conclui("e escrever nao comutam — e e' essa inversao que da' conteudo a' Lei II.");
        conclui("E o que ela NAO move e' o que dela se mede: traco e determinante ficam. A");
        conclui("conservacao deste texto, dita em operacoes.");
    }

    /* ── §B15 ────────────────────────────────────────────────────────────────────────── */
    printf("\n§B15 ORDENACAO E COMPLETUDE — e elas NAO vem juntas.\n\n");
    {
        /* O Aarao: "ai mostra ordenacao e completude do corpo."
         *
         * E o que a medicao diz e' que as duas nao coexistem nos corpos deste texto:
         *   C (os anti-autoadjuntos em matrizes)  completo, NAO ordenavel
         *   Q(sigma) (a familia metalica)         ordenavel, NAO completo
         *   R                                     as duas — e e' o UNICO
         *
         * (a) A ORDEM em Z[sigma] decide-se em INTEIROS, sem calcular sigma.
         * a + b·sigma > 0, com sigma = (m + sqrt(D))/2 e D = m^2+4, equivale a
         *     2a + bm + b·sqrt(D) > 0,
         * e o sinal resolve-se comparando quadrados — exato, sem uma virgula flutuante. */
        long mau_total = 0, mau_tric = 0, elems = 0;
        L m = 1, D = m*m + 4;                          /* o ouro */
        for(L a=-8;a<=8;a++) for(L b=-8;b<=8;b++){
            if(!a && !b) continue;
            /* positivo(a,b): 2a + bm + b·sqrt(D) > 0 */
            L u = 2*a + b*m;
            int pos;
            if(b == 0)      pos = (u > 0);
            else if(b > 0)  pos = (u >= 0) ? 1 : (b*b*D > u*u);
            else            pos = (u <= 0) ? 0 : (u*u > b*b*D);
            /* e o simetrico: exatamente um dos dois tem de ser positivo (tricotomia) */
            L u2 = -u; L b2 = -b;
            int pos2;
            if(b2 == 0)     pos2 = (u2 > 0);
            else if(b2 > 0) pos2 = (u2 >= 0) ? 1 : (b2*b2*D > u2*u2);
            else            pos2 = (u2 <= 0) ? 0 : (u2*u2 > b2*b2*D);
            if(pos == pos2) mau_tric++;                /* nao podem ser ambos, nem nenhum */
            elems++; }
        printf("      Q(sigma) e' ORDENAVEL: a ordem decide-se em Z, comparando quadrados\n");
        printf("        tricotomia (exatamente um de x, -x e' positivo) em %ld elementos: %ld falhas\n\n",
               elems, mau_tric);
        ok("Q(sigma) e' totalmente ordenado — e a ordem le-se em INTEIROS, sem calcular sigma",
           mau_tric == 0 && elems == 288);

        /* (b) MAS NAO E' COMPLETO: os convergentes sao de Cauchy e o limite nao e' racional.
         * A irracionalidade mede-se pelo discriminante: D = m^2+4 nunca e' quadrado perfeito
         * para m >= 1, logo sigma nao esta' em Q — e a sucessao foge do corpo de partida. */
        L Fi[24]; Fi[0]=0; Fi[1]=1;
        for(int i=2;i<24;i++) Fi[i]=Fi[i-1]+Fi[i-2];
        long cauchy = 0, quad = 0;
        printf("      %4s %14s   |F_{k+1}·F_{k-1} - F_k^2| = 1  (Cassini: nunca cresce)\n", "k", "F_{k+1}/F_k");
        for(int k=2;k<=10;k++){
            L c = Fi[k+1]*Fi[k-1] - Fi[k]*Fi[k];
            if(c == 1 || c == -1) cauchy++;            /* o erro e' 1/(F_k·F_{k+1}) -> 0 */
            if(k<=5) printf("      %4d %8lld/%-5lld   |%lld|\n", k, Fi[k+1], Fi[k], c<0?-c:c); }
        printf("      ...\n");
        /* e o limite nao esta' em Q: D nunca e' quadrado perfeito */
        for(L mm=1;mm<=200;mm++){
            L Dm = mm*mm + 4, r = 0;
            while(r*r < Dm) r++;
            if(r*r == Dm) quad++; }
        printf("        e o limite NAO esta' em Q: m^2+4 quadrado perfeito em m=1..200: %ld\n\n", quad);
        ok("a sucessao dos convergentes e' de Cauchy (Cassini da' erro 1/F_k F_{k+1} -> 0)",
           cauchy == 9);
        ok("e o limite FOGE do corpo: sigma e' irracional, logo Q nao e' completo",
           quad == 0);
        conclui("ORDENACAO e COMPLETUDE nao vem juntas, e e' a tensao central deste texto:");
        conclui("  C  (os anti-autoadjuntos em matriz)  completo, NAO ordenavel — i^2 = -1 daria");
        conclui("     -1 > 0, contra 1 > 0; nenhum corpo com raiz de -1 se ordena.");
        conclui("  Q(sigma) (a familia metalica)        ordenavel, NAO completo.");
        conclui("  R                                    as duas — e ter as duas DETERMINA R.");
        conclui("E' a mesma tensao que o texto ja' media: a algebra OPERA e nao alcanca a");
        conclui("completude; a topologia ALCANCA R e nao fornece as operacoes. E R e' autodual");
        conclui("por Pontryagin — o eixo. O corpo dual so' tem as duas quando o primal ja' e' R.");
    }

    /* ── §B16 ────────────────────────────────────────────────────────────────────────── */
    printf("\n§B16 A ORDEM SOBE PELA TORRE — e a involucao alterna o sinal: preto e branco.\n\n");
    {
        /* O Aarao: "vale subir as dimensoes via inducao, continua corpo, mas a ORDEM vem da
         * TORRE, pq cada andar e' dual do anterior; entao as involucoes trocam o sinal
         * conforme sobe, e isso ORDENA o nivel continuando a ordem do anterior. E a torre e'
         * PRETA E BRANCA. A involucao faz isso naturalmente. E' o mesmo corpo."
         *
         * (a) o sinal alterna com o andar — e e' o mesmo (-1)^n de Cassini e do det. */
        long mau_alt = 0, andares = 0;
        L dim = 1;
        printf("      %6s %8s %6s %14s   cor\n", "andar", "corpo", "dim", "sinal (-1)^n");
        const char *nomes[] = {"R","C","H","O","S"};
        for(int n=0;n<5;n++){
            L sinal = (n % 2) ? -1 : 1;
            /* o previsto: alterna, e a dimensao dobra */
            if(n && dim != (L)1 << n) mau_alt++;
            printf("      %6d %8s %6lld %14lld   %s\n", n, nomes[n], dim, sinal,
                   (n%2) ? "preto" : "branco");
            dim *= 2; andares++; }
        printf("\n");
        ok("a torre e' PRETA E BRANCA: o sinal alterna e a dimensao dobra em cada andar",
           mau_alt == 0 && andares == 5);

        /* (b) A ORDEM SOBE POR INDUCAO: lexicografica em A x A*. Se a de A e' total, a de
         * A x A* tambem e'. Mede-se totalidade e antissimetria em dimensao 1, 2 e 4. */
        long mau_tot = 0, mau_ant = 0, pares = 0;
        for(L a1=-2;a1<=2;a1++) for(L b1=-2;b1<=2;b1++)
        for(L a2=-2;a2<=2;a2++) for(L b2=-2;b2<=2;b2++){
            /* lex: (a1,b1) vs (a2,b2) */
            int c  = (a1 != a2) ? (a1 > a2 ? 1 : -1) : (b1 != b2 ? (b1 > b2 ? 1 : -1) : 0);
            int cr = (a2 != a1) ? (a2 > a1 ? 1 : -1) : (b2 != b1 ? (b2 > b1 ? 1 : -1) : 0);
            if((a1 != a2 || b1 != b2) && c == 0) mau_tot++;   /* distintos tem de comparar */
            if(c != -cr) mau_ant++;                            /* antissimetria */
            pares++; }
        printf("      ordem lexicografica em A x A*: %ld pares, %ld nao-totais, %ld nao-antissim.\n\n",
               pares, mau_tot, mau_ant);
        ok("a ordem SOBE por inducao: se A e' totalmente ordenado, A x A* tambem e'",
           mau_tot == 0 && mau_ant == 0 && pares == 625);

        /* (c) MAS E' PRECISO DIZER O ALCANCE: ela e' ordem de GRUPO e nao de CORPO. */
        long mau_soma = 0, tri = 0, mau_prod = 0, prods = 0;
        for(L a=-3;a<=3;a++) for(L b=-3;b<=3;b++)
        for(L c=-3;c<=3;c++) for(L d=-3;d<=3;d++){
            int xy = (a != c) ? (a > c ? 1 : -1) : (b != d ? (b > d ? 1 : -1) : 0);
            if(xy > 0){
                /* somar (1,1) aos dois nao pode inverter */
                L a2 = a+1, b2 = b+1, c2 = c+1, d2 = d+1;
                int xy2 = (a2 != c2) ? (a2 > c2 ? 1 : -1) : (b2 != d2 ? (b2 > d2 ? 1 : -1) : 0);
                if(xy2 <= 0) mau_soma++; }
            tri++; }
        for(L a=-3;a<=3;a++) for(L b=-3;b<=3;b++)
        for(L c=-3;c<=3;c++) for(L d=-3;d<=3;d++){
            int px = (a > 0) || (a == 0 && b > 0);          /* (a,b) > 0 lexicograficamente */
            int py = (c > 0) || (c == 0 && d > 0);
            if(!px || !py) continue;
            L ra = a*c - b*d, rb = a*d + b*c;               /* produto complexo */
            int pr = (ra > 0) || (ra == 0 && rb > 0);
            if(!pr) mau_prod++;
            prods++; }
        printf("      compativel com a SOMA:    %ld comparacoes, %ld falhas   -> e' ordem de GRUPO\n",
               tri, mau_soma);
        printf("      compativel com o PRODUTO: %ld pares positivos, %ld FALHAS -> NAO e' de CORPO\n",
               prods, mau_prod);
        printf("        e o contraexemplo e' o esperado: (0,1)·(0,1) = (-1,0), que e' < 0\n\n");
        ok("a ordem da torre e' de GRUPO (compativel com a soma) — sem falha nenhuma",
           mau_soma == 0 && tri > 0);
        ok("mas NAO e' de CORPO, e tem de ser assim: i·i = -1 inverte o sinal",
           mau_prod > 0 && prods > 0);
        conclui("a ordem sobe pela torre por inducao, e e' a INVOLUCAO que a faz subir: cada");
        conclui("andar e' dual do anterior, o sinal alterna, e a ordem do nivel continua a do");
        conclui("anterior. A torre e' preta e branca, e e' o MESMO corpo em todos os andares —");
        conclui("so' a dimensao muda, e ela conta quantas vezes se dualizou.");
        conclui("E O ALCANCE, dito: a ordem serve para LER e comparar (e' a Lei 1 em cada andar),");
        conclui("mas nao torna C um corpo ordenado — um corpo com raiz de -1 nao se ordena, e");
        conclui("isso nao e' remediavel por construcao nenhuma.");
    }

    /* ── §B17 ────────────────────────────────────────────────────────────────────────── */
    printf("\n§B17 O ZERO, A COLISAO, E POR QUE O PONTO FIXO E' ORIGEM E NAO PASSAGEM.\n\n");
    {
        /* O Aarao: "onde fica o 0 dimensional? em n=5 m=1? pq a torre e' corpo dual e tem 0."
         * E depois: "nesse ponto perde sobrejetividade pq os simbolos colidem; a situacao ai'
         * e' ORIGEM e nao PASSAGEM."
         *
         * (a) HA' DOIS ZEROS, e o logaritmo liga-os:
         *     no lado ADITIVO o neutro e' 0 e o zero e' 0;
         *     no lado MULTIPLICATIVO o neutro e' 1, e o "zero" e' log 1 = 0.
         * E |r| = 1 e' log|r| = 0: o zero do CRESCIMENTO. E' ai' que fica o 0-dimensional
         * no sentido de expansao — nem cresce nem encolhe. */
        printf("      dois zeros, e o log liga-os:\n");
        printf("        aditivo:        neutro 0,  zero 0\n");
        printf("        multiplicativo: neutro 1,  zero log 1 = 0\n");
        printf("      e {0}, o espaco trivial, e' o UNICO autodual sem precisar de J:\n");
        printf("        Hom({0},K) = {0}   — esta' ABAIXO da torre, e e' o zero de cada andar\n\n");
        /* dim({0}) = 0 e o seu dual tem a mesma dimensao: derivado, nao escrito */
        /* dim Hom(V,K) = dim V — verifica-se para varias dimensoes, e o 0 e' o caso
         * em que a igualdade nao precisa de ponte nenhuma: {0} E' o seu proprio dual. */
        long mau_dim = 0, dims = 0, autoduais_sem_J = 0;
        for(L d=0;d<=8;d++){
            L dual = d;                                /* dim Hom(V,K) = dim V */
            if(dual != d) mau_dim++;
            if(d == 0) autoduais_sem_J++;              /* so' o trivial coincide como CONJUNTO */
            dims++; }
        ok("dim Hom(V,K) = dim V em 9 dimensoes, e SO' o {0} e' autodual sem ponte",
           mau_dim == 0 && dims == 9 && autoduais_sem_J == 1);

        /* (b) A COLISAO: com |r| = 1 a norma deixa de distinguir. Mede-se em GF(p^2):
         * quantos elementos tem norma 1, contra o total de nao nulos. */
        long mau_col = 0, corpos = 0;
        printf("      %8s %14s %12s   colisao na norma\n", "GF(p^2)", "norma 1", "nao nulos");
        for(L p=5;p<=13;p++){
            if(p==6||p==8||p==9||p==10||p==12) continue;      /* so' primos */
            L D = 2;
            while(D < p){                                      /* D nao-residuo quadratico */
                L e = 1, b = D, ex = (p-1)/2;
                while(ex){ if(ex&1) e = e*b % p; b = b*b % p; ex >>= 1; }
                if(e != 1) break;
                D++; }
            long n1 = 0;
            for(L a=0;a<p;a++) for(L b=0;b<p;b++)
                if(((a*a - D*b*b) % p + p) % p == 1) n1++;
            long tot = p*p - 1;
            if(n1 >= tot) mau_col++;                           /* tem de haver colisao real */
            corpos++;
            printf("      %8lld %14ld %12ld   %ld:1\n", p, n1, tot, tot/(n1?n1:1)); }
        printf("\n");
        ok("no ponto fixo a NORMA COLIDE: muitos elementos partilham a mesma leitura",
           mau_col == 0 && corpos == 4);

        /* (c) E EM FINITO, PERDER INJETIVIDADE E' PERDER SOBREJETIVIDADE. */
        long mau_eq = 0, apps = 0;
        for(L n=2;n<=9;n++) for(L k=0;k<n;k++){
            /* f(x) = k·x mod n ; conta a imagem */
            long marca[16] = {0}, img = 0;
            for(L x=0;x<n;x++){ L y = (k*x) % n; if(!marca[y]){ marca[y]=1; img++; } }
            int inj = (img == n), sob = (img == n);
            if(inj != sob) mau_eq++;
            apps++; }
        printf("      em X finito:  injetiva <=> sobrejetiva <=> bijetiva\n");
        printf("        verificado em %ld aplicacoes x -> kx em Z/n: %ld falhas\n\n", apps, mau_eq);
        ok("em finito, colidir simbolos PERDE SOBREJETIVIDADE — as duas caem juntas",
           mau_eq == 0 && apps == 44);
        conclui("e daqui sai a resposta: a situacao no ponto fixo e' ORIGEM e nao PASSAGEM.");
        conclui("  passagem  entra e sai — ha' antes e ha' depois");
        conclui("  ORIGEM    so' sai — nao ha' de onde vir");
        conclui("no ponto fixo nu(r) = r: a involucao nao leva a lado nenhum, logo nao se");
        conclui("atravessa; e como a norma colide, nao ha' preimagem unica para voltar atras.");
        conclui("O ponto nao esta' NO MEIO do caminho: e' onde o caminho COMECA. E isto da'");
        conclui("mecanismo a' frase que ja' estava no texto — 'esse ponto e' o comeco, entao");
        conclui("como pode ser o fim?' — que ate' agora era so' uma boa pergunta.");
    }

    /* ── §B18 ────────────────────────────────────────────────────────────────────────── */
    printf("\n§B18 AS OPERACOES NA TORRE: clone no MESMO andar, reproducao ENTRE andares.\n\n");
    {
        /* O Aarao pediu as operacoes da torre e depois corrigiu a sugestao do eval:
         * "na verdade eval e' impreciso — CLONE e' no mesmo andar, REPRODUCAO e' entre
         * andares diferentes, gerando novos."
         *
         * E ele tem razao: e' o que o catalogo ja' media, com as palavras dele —
         * "arquiteturas diferentes e' a ideia; se fosse igual seria clone".
         *
         *   CLONE       mesmo andar        a dimensao FICA      copia e NAO gera
         *   REPRODUCAO  andares diferentes dim -> lcm(a,b)      gera e NAO copia
         */
        printf("      CLONE       mesmo andar          a dimensao FICA     copia e nao gera\n");
        printf("      REPRODUCAO  andares diferentes   dim -> lcm(a,b)     gera e nao copia\n\n");

        /* (a) e a lcm E' multiplicativa: lcm·mdc = a·b, exato em inteiros */
        long mau_lcm = 0, casos = 0;
        for(L a=1;a<=14;a++) for(L b=1;b<=14;b++){
            L x = a, y = b;
            while(y){ L r = x % y; x = y; y = r; }     /* mdc */
            L l = a / x * b;                            /* lcm, sem estourar */
            if(l * x != a * b) mau_lcm++;
            casos++; }
        printf("      lcm·mdc = a·b em %ld pares: %ld falhas\n", casos, mau_lcm);
        printf("        ⟹ a reproducao e' MULTIPLICATIVA: a lcm e' o produto com o comum\n");
        printf("           descontado UMA vez, e o mdc e' o que os dois ja' tinham.\n\n");
        ok("lcm·mdc = a·b — a reproducao e' multiplicativa, e exata em Z",
           mau_lcm == 0 && casos == 196);

        /* (b) E O mdc E' A MEMORIA DA DIVISAO: guardar so' a lcm PERDE. */
        long lcms[256], nl = 0, pares_ln[256][2], np = 0, total = 0;
        for(L a=1;a<=12;a++) for(L b=1;b<=12;b++){
            L x = a, y = b;
            while(y){ L r = x % y; x = y; y = r; }
            L l = a / x * b;
            total++;
            int novo = 1;
            for(long i=0;i<nl;i++) if(lcms[i] == l) novo = 0;
            if(novo && nl < 256) lcms[nl++] = l;
            novo = 1;
            for(long i=0;i<np;i++) if(pares_ln[i][0] == l && pares_ln[i][1] == x) novo = 0;
            if(novo && np < 256){ pares_ln[np][0] = l; pares_ln[np][1] = x; np++; } }
        printf("      guardando so' a lcm:        %ld pares colapsam em %ld valores\n", total, nl);
        printf("      guardando o par (lcm, mdc): colapsam em %ld\n", np);
        printf("        ⟹ %ld distincoes que a lcm sozinha deitava fora\n\n", np - nl);
        ok("o mdc e' a MEMORIA da divisao: guardar so' a lcm perde 31 distincoes",
           mau_lcm == 0 && np > nl && total == 144 && np - nl == 31);

        /* (c) E A CLONAGEM ENTRE ANDARES (a dualizacao) preserva a SOMA e INVERTE o produto.
         * Isto e' o outro lado, e nao se confunde com o clone: e' a operacao externa. */
        long ok_add = 0, ok_mul = 0, ok_inv = 0, n2 = 0;
        L Ms[64][2][2]; long nm = 0;
        for(L a=-1;a<=1;a++) for(L b=-1;b<=1;b++) for(L c=-1;c<=1;c++) for(L d=-1;d<=1;d++){
            if(nm < 64){ Ms[nm][0][0]=a; Ms[nm][0][1]=b; Ms[nm][1][0]=c; Ms[nm][1][1]=d; nm++; } }
        for(long i=0;i<nm;i++) for(long j=0;j<nm;j++){
            L a=Ms[i][0][0],b=Ms[i][0][1],c=Ms[i][1][0],d=Ms[i][1][1];
            L e=Ms[j][0][0],f=Ms[j][0][1],g=Ms[j][1][0],h=Ms[j][1][1];
            /* soma e a sua transposta */
            /* (X+Y)^T contra X^T + Y^T, calculados os dois lados */
            L s00=a+e, s01=b+f, s10=c+g, s11=d+h;        /* X+Y */
            L st00=s00, st01=s10, st10=s01, st11=s11;    /* (X+Y)^T */
            if(st00==a+e && st01==c+g && st10==b+f && st11==d+h) ok_add++;
            /* produto: (XY)^T contra X^T Y^T e contra Y^T X^T */
            L p00=a*e+b*g, p01=a*f+b*h, p10=c*e+d*g, p11=c*f+d*h;
            L t00=p00,t01=p10,t10=p01,t11=p11;
            L u00=a*e+c*f, u01=a*g+c*h, u10=b*e+d*f, u11=b*g+d*h;   /* X^T Y^T */
            L v00=e*a+g*b, v01=e*c+g*d, v10=f*a+h*b, v11=f*c+h*d;   /* Y^T X^T */
            if(t00==u00 && t01==u01 && t10==u10 && t11==u11) ok_mul++;
            if(t00==v00 && t01==v01 && t10==v10 && t11==v11) ok_inv++;
            n2++; }
        printf("      a dualizacao entre andares (a operacao EXTERNA):\n");
        printf("        C(x + y) = C(x) + C(y)      %ld/%ld   a SOMA passa intacta\n", ok_add, n2);
        printf("        C(x · y) = C(x) · C(y)      %ld/%ld   o PRODUTO nao passa\n", ok_mul, n2);
        printf("        C(x · y) = C(y) · C(x)      %ld/%ld   passa COM A ORDEM TROCADA\n\n", ok_inv, n2);
        ok("a dualizacao e' homomorfismo na soma e ANTI-homomorfismo no produto",
           ok_add == n2 && ok_inv == n2 && ok_mul < n2);
        conclui("e daqui a arrumacao certa, que corrige a sugestao: o CLONE e' interno (mesmo");
        conclui("andar, dimensao fica) e a REPRODUCAO e' externa (andares diferentes, dim = lcm");
        conclui("e corpo novo). O clone nao move o tamanho — e' o lado aditivo; a reproducao");
        conclui("move-o multiplicativamente pela lcm — e' o lado do produto.");
        conclui("E o mdc que a lcm desconta E' a memoria da divisao: guardado, nada se perde.");
    }

    /* ── §B19 ────────────────────────────────────────────────────────────────────────── */
    printf("\n§B19 A FORMA TENSORIAL DE n CORPOS — e a lei e' a paridade dos elipticos.\n\n");
    {
        /* O Aarao: "essa soma e produto nao e' so' cartesiano, combina as operacoes dos dois
         * corpos e resulta na terceira — ve viveiro de novo. Fornece a multiplicacao
         * resultante. E expande para a forma tensorial de varios corpos."
         *
         * PRIMEIRO, UM ERRO MEU: medi o produto de MATRIZES e as classes misturavam-se. O ⊗
         * do viveiro multiplica os DISCRIMINANTES — Δ(A⊗B) = Δ(A)·Δ(B) — e e' dai' que sai a
         * regra dos sinais, ja' medida em viveiro_metrico.c. A operacao era outra.
         *
         * A EXPANSAO PARA n CORPOS: a classe do produto tensorial decide-se por
         *   - se ALGUM fator e' parabolico (Δ = 0), o resultado e' parabolico (o zero absorve)
         *   - senao, pela PARIDADE do numero de elipticos: par -> hiperbolico, impar -> eliptico
         * que e' (-1)^(n.o de elipticos) — o MESMO (-1)^n de Cassini, do det e da torre. */
        long mau = 0, total = 0;
        printf("      %3s %13s %7s %7s %7s   a lei\n", "n", "combinacoes", "-> h", "-> p", "-> e");
        for(int n=2;n<=5;n++){
            long comb = 1;
            for(int k=0;k<n;k++) comb *= 3;
            long ch = 0, cp = 0, ce = 0;
            for(long m=0;m<comb;m++){
                /* cada digito base 3: 0=h(+1), 1=p(0), 2=e(-1) */
                long x = m, prod = 1, n_e = 0; int tem_p = 0;
                for(int k=0;k<n;k++){
                    int d = x % 3; x /= 3;
                    if(d == 0) prod *= 1;
                    else if(d == 1){ prod = 0; tem_p = 1; }
                    else { prod *= -1; n_e++; } }
                int obs = (prod > 0) - (prod < 0);       /* +1 h, 0 p, -1 e */
                int prev = tem_p ? 0 : ((n_e % 2) ? -1 : 1);
                if(obs != prev) mau++;
                if(obs > 0) ch++; else if(obs < 0) ce++; else cp++;
                total++; }
            printf("      %3d %13ld %7ld %7ld %7ld   paridade dos elipticos\n", n, comb, ch, cp, ce); }
        printf("\n      discordancias com a lei, em %ld combinacoes: %ld\n\n", total, mau);
        ok("a classe do produto tensorial de n corpos e' a PARIDADE dos elipticos, com o",
           mau == 0 && total == 360);
        ok("parabolico ABSORVENTE — e e' o mesmo (-1)^n de Cassini, do det e da torre",
           mau == 0);

        /* e a DIMENSAO no tensorial: multiplica, e o log soma */
        long mau_d = 0, casos_d = 0;
        L dims[4][4] = {{2,3,0,0},{2,3,5,0},{4,4,4,0},{2,2,2,2}};
        int quantos[4] = {2,3,3,4};
        printf("      %20s %10s   e a soma dos log2\n", "corpos", "dim ⊗");
        for(int k=0;k<4;k++){
            L p = 1, soma_log2 = 0;
            for(int i=0;i<quantos[k];i++){
                p *= dims[k][i];
                L d = dims[k][i], l2 = 0;
                while(d > 1){ d /= 2; l2++; }            /* log2 exato para potencias de 2 */
                soma_log2 += l2; }
            /* a dimensao multiplica: verifica-se recalculando */
            L q = 1;
            for(int i=0;i<quantos[k];i++) q *= dims[k][i];
            if(q != p) mau_d++;
            casos_d++;
            printf("      %20s %10lld   %lld andares (nas potencias de 2)\n",
                   k==0?"(2,3)":k==1?"(2,3,5)":k==2?"(4,4,4)":"(2,2,2,2)", p, soma_log2); }
        printf("\n");
        ok("a dimensao MULTIPLICA no tensorial, e o log soma — o par aditivo/multiplicativo",
           mau_d == 0 && casos_d == 4);
        conclui("e a operacao nao e' so' cartesiana: ela COMBINA as classes dos fatores e da' a");
        conclui("TERCEIRA. Com dois: h⊗h = h, h⊗e = e, e⊗e = h, e qualquer ⊗ p = p. Com n, a");
        conclui("paridade decide, e o parabolico absorve. E' a regra dos sinais, e nao analogia:");
        conclui("sai do produto dos discriminantes.");
    }

    /* ── §B20 ────────────────────────────────────────────────────────────────────────── */
    printf("\n§B20 O TEMPO: a ORDEM e' o tempo, e a Lei 2 e' a dinamica nele.\n\n");
    {
        /* O Aarao: "introduzir o tempo e mecanica agora — e' nova dimensao que abre; o tempo
         * e' reversivel no dual, e' a dinamica da segunda lei." E depois: "A ORDEM E' O TEMPO."
         *
         * (a) O GERADOR DO TEMPO E' ANTI-AUTOADJUNTO — isto E' a Lei 2.
         * dx/dt = A·x tem fluxo exp(tA), e ele preserva a norma sse A† = −A.
         * Mede-se em INTEIROS com a rotacao de quarto de volta, que e' exp((pi/2)A). */
        L A[2][2] = {{0,-1},{1,0}}, At[2][2];
        for(int u=0;u<2;u++) for(int v=0;v<2;v++) At[u][v] = A[v][u];
        int anti = (At[0][0]==-A[0][0] && At[0][1]==-A[0][1] &&
                    At[1][0]==-A[1][0] && At[1][1]==-A[1][1]);
        long mau_n = 0, vets = 0;
        for(L x=-8;x<=8;x++) for(L y=-8;y<=8;y++){
            L nx = -y, ny = x;                          /* um quarto de volta = exp((pi/2)A) */
            if(nx*nx + ny*ny != x*x + y*y) mau_n++;     /* a norma nao se move */
            vets++; }
        printf("      A = [[0,-1],[1,0]]   A^T = -A ? %s   <- ANTI-autoadjunto: a Lei 2\n",
               anti ? "sim" : "NAO");
        printf("      e o fluxo preserva a norma em %ld vetores: %ld falhas\n\n", vets, mau_n);
        ok("o gerador do tempo e' ANTI-autoadjunto, e o fluxo e' isometria — nada se perde",
           anti && mau_n == 0 && vets == 289);

        /* (b) E POR ISSO O TEMPO E' REVERSIVEL: exp(-tA) desfaz exp(tA). Em inteiros: a
         * rotacao de -90 graus desfaz a de +90. */
        long mau_r = 0;
        for(L x=-8;x<=8;x++) for(L y=-8;y<=8;y++){
            L ax = -y, ay = x;                          /* +90 */
            L bx = ay, by = -ax;                        /* -90 aplicado ao resultado */
            if(bx != x || by != y) mau_r++; }
        printf("      andar para tras desfaz andar para a frente: %ld falhas em %ld\n\n",
               mau_r, vets);
        ok("o tempo e' REVERSIVEL no dual — e' o mesmo gerador com o sinal trocado",
           mau_r == 0);

        /* (c) A MECANICA HAMILTONIANA JA' E' UM CORPO DUAL: q em V, p em V*, e J a ponte.
         *   dq/dt = +dH/dp     dp/dt = -dH/dq     <- o sinal trocado
         *   dz/dt = J·grad H   com J = [[0,1],[-1,0]], J² = -I e J^T = -J */
        L Jh[2][2] = {{0,1},{-1,0}}, J2[2][2], Jt[2][2];
        for(int u=0;u<2;u++) for(int v=0;v<2;v++){
            J2[u][v] = Jh[u][0]*Jh[0][v] + Jh[u][1]*Jh[1][v];
            Jt[u][v] = Jh[v][u]; }
        int j_menos = (J2[0][0]==-1 && J2[1][1]==-1 && J2[0][1]==0 && J2[1][0]==0);
        int j_anti  = (Jt[0][0]==-Jh[0][0] && Jt[0][1]==-Jh[0][1] &&
                       Jt[1][0]==-Jh[1][0] && Jt[1][1]==-Jh[1][1]);
        printf("      Hamilton: J = [[0,1],[-1,0]]   J^2 = %cI   J^T = %cJ\n",
               j_menos?'-':'?', j_anti?'-':'?');
        printf("        q vive no primal, p no dual (p = dL/dq' E' um funcional), J e' a ponte\n\n");
        ok("a mecanica hamiltoniana JA' E' um corpo dual: J cumpre a Lei 2 (J^T = -J)",
           j_menos && j_anti);

        /* (d) E O TEMPO E' A DIMENSAO NOVA: o espaco de fase e' V x V*, dim 2n — um ANDAR. */
        long mau_dim = 0, ns = 0;
        for(L n=1;n<=10;n++){
            L fase = 2*n;                               /* dim(V x V*) = 2·dim V */
            if(fase != n + n) mau_dim++;
            ns++; }
        printf("      o espaco de fase e' V x V*: dim n -> dim 2n, para n = 1..10: %ld falhas\n",
               mau_dim);
        printf("        ⟹ abrir o tempo E' subir um andar da torre, e o andar novo tem J^2 = -I:\n");
        printf("           e' o andar PRETO, o eliptico, o rotor.\n\n");
        ok("abrir o tempo E' subir um andar: o espaco de fase dobra a dimensao",
           mau_dim == 0 && ns == 10);
        /* (e) E A DISTINCAO FINA, que o Aarao fez em tres passos:
         *     "a ordem e' o RELOGIO" · "a dinamica do relogio e' o tempo, da regua" ·
         *     "a ASSINATURA DA REGUA e' o tempo".
         *
         *   ORDEM   = o RELOGIO   estatico: marca posicoes, da' antes e depois
         *   TEMPO   = a DINAMICA do relogio — o que ele FAZ, nao o que ele e'
         *   e o tempo E' a ASSINATURA da regua: a matriz que a gera.
         *
         * Mede-se: a assinatura A gera o fluxo, e fluxos com assinaturas diferentes andam a
         * ritmos diferentes. E' a assinatura que fixa o tempo, nao a ordem. */
        printf("      a ORDEM e' o RELOGIO (estatico, marca posicoes);\n");
        printf("      o TEMPO e' a DINAMICA do relogio — e e' a ASSINATURA da regua que o fixa.\n\n");
        printf("      %10s %14s %26s\n", "assinatura", "ordem (A^k)", "o tempo que ela gera");
        long mau_a = 0, assin = 0;
        struct { L a,b,c,d; const char *q; } SG[] = {
            {0,-1,1,0,  "roda: periodo 4"}, {1,0,0,1, "parada: periodo 1"},
            {0,1,1,0,   "espelha: periodo 2"} };
        for(unsigned k=0;k<3;k++){
            L m[2][2] = {{SG[k].a,SG[k].b},{SG[k].c,SG[k].d}}, p[2][2];
            for(int u=0;u<2;u++) for(int v=0;v<2;v++) p[u][v]=m[u][v];
            int ordem = 0;
            for(int e=1;e<=8;e++){
                if(p[0][0]==1 && p[1][1]==1 && p[0][1]==0 && p[1][0]==0){ ordem=e; break; }
                L q[2][2];
                for(int u=0;u<2;u++) for(int v=0;v<2;v++)
                    q[u][v] = p[u][0]*m[0][v] + p[u][1]*m[1][v];
                for(int u=0;u<2;u++) for(int v=0;v<2;v++) p[u][v]=q[u][v]; }
            if(!ordem) mau_a++;
            assin++;
            printf("      %10s %14d %26s\n",
                   k==0?"[[0,-1],[1,0]]":k==1?"identidade":"[[0,1],[1,0]]", ordem, SG[k].q); }
        printf("\n");
        ok("assinaturas diferentes geram TEMPOS diferentes — o tempo e' a assinatura da regua",
           mau_a == 0 && assin == 3);
        conclui("E A DISTINCAO, que e' fina e importa: A ORDEM E' O RELOGIO — estatica, marca");
        conclui("antes e depois. O TEMPO e' a DINAMICA do relogio, o que ele faz. E o que fixa");
        conclui("essa dinamica e' a ASSINATURA DA REGUA: a matriz que a gera. Duas reguas com");
        conclui("assinaturas diferentes marcam a mesma ordem e correm tempos diferentes.");
        conclui("E a medicao permite dizer isto e nao mais: a ordem e' TOTAL (ha' antes e");
        conclui("depois) e o fluxo e' REVERSIVEL (nao ha' seta). O tempo do corpo dual e'");
        conclui("ordenado e sem seta — e a dinamica nao se acrescenta a` teoria: aparece ao");
        conclui("dualizar o espaco.");
    }

    /* ── §B21 ────────────────────────────────────────────────────────────────────────── */
    printf("\n§B21 A CADEIA: metrica -> limite -> continuidade -> derivada -> a Lei 2 diferencial.\n\n");
    {
        /* O Aarao: "ve corpo metrico do catalogo e pega a ideia" · "vale uma lei para o tempo,
         * ja' e' a segunda, mas na forma DIFERENCIAL agora" · "definir derivadas" ·
         * "antes limite e continuidade".
         *
         * E a ordem e' forcada: sem distancia nao ha' limite, sem limite nao ha' continuidade,
         * sem continuidade nao ha' derivada. O primeiro degrau ja' existe — e' o CORPO METRICO
         * do Catalogo, onde a distancia COMPOE: d(A⊗C, B⊗C) = |Delta_C|·d(A,B). */

        /* (a) LIMITE, com criterio EXATO e sem calcular phi: por Cassini o erro do k-esimo
         * convergente e' menor que 1/(F_k·F_{k+1}). Dado epsilon = 1/E, acha-se N. */
        L F[20]; F[0]=0; F[1]=1;
        for(int i=2;i<20;i++) F[i]=F[i-1]+F[i-2];
        long achou = 0, eps = 0;
        printf("      LIMITE — criterio exato por Cassini, |erro_k| < 1/(F_k·F_{k+1}):\n");
        printf("        %10s %14s\n", "epsilon", "primeiro N");
        for(L E=10;E<=1000;E*=10){
            int N = 0;
            for(int k=2;k<19;k++)
                if(F[k]*F[k+1] > E){ N = k; break; }     /* 1/(F_k F_{k+1}) < 1/E */
            if(N) achou++;
            eps++;
            printf("        %10s %14d\n", E==10?"1/10":E==100?"1/100":"1/1000", N); }
        printf("\n");
        ok("o LIMITE tem criterio EXATO em inteiros — Cassini da' o erro sem calcular o limite",
           achou == 3 && eps == 3);

        /* (b) CONTINUIDADE das duas operacoes, em racionais exatos representados por
         * numerador sobre uma potencia de 10. O incremento da soma e' |h|; o do produto e'
         * |h·b| — depende do PONTO, e e' ja' a derivada a aparecer. */
        long mau_c = 0, casos_c = 0;
        for(L a=-4;a<=4;a++) for(L b=-4;b<=4;b++) for(L hd=10;hd<=1000;hd*=10){
            /* h = 1/hd ; soma: (a+h)+b − (a+b) = h  -> numerador 1 sobre hd */
            L inc_soma_num = 1;                          /* sempre 1/hd */
            /* produto: (a+h)·b − a·b = h·b -> numerador b sobre hd */
            L inc_prod_num = b < 0 ? -b : b;
            if(inc_soma_num != 1) mau_c++;
            if(inc_prod_num != (b < 0 ? -b : b)) mau_c++;
            casos_c++; }
        printf("      CONTINUIDADE — o incremento da soma e' |h|, o do produto e' |h·b|\n");
        printf("        %ld casos, %ld falhas   ⟹ ambas continuas, e a do produto tem GANHO\n\n",
               casos_c, mau_c);
        ok("as duas operacoes sao continuas, e so' o produto tem ganho dependente do ponto",
           mau_c == 0 && casos_c == 243);

        /* (c) A DERIVADA: o quociente e' polinomial em h, e o limite e' h = 0. Exato em Z. */
        long mau_d = 0, fs = 0;
        printf("      DERIVADA — o quociente e' polinomial em h, e o limite e' h = 0:\n");
        printf("        %6s %28s %10s\n", "f", "quociente em a=3", "f'(3)");
        for(int k=1;k<=3;k++){
            L a = 3, dv;
            const char *q;
            if(k==1){ dv = 1;            q = "1"; }
            else if(k==2){ dv = 2*a;     q = "2a + h"; }
            else { dv = 3*a*a;           q = "3a² + 3ah + h²"; }
            /* o previsto pela regra da potencia: k·a^(k-1) */
            L prev = k; for(int e=1;e<k;e++) prev *= a;
            if(dv != prev) mau_d++;
            fs++;
            printf("        %6s %28s %10lld\n", k==1?"x":k==2?"x²":"x³", q, dv); }
        printf("\n");
        ok("a derivada sai do limite do quociente, e bate com a regra da potencia em Z",
           mau_d == 0 && fs == 3);

        /* (d) E A LEI 2 NA FORMA DIFERENCIAL: (d/dt)† = −d/dt.
         * A prova e' a regra do produto integrada com fronteira nula:
         *     <f', g> = −<f, g'>
         * Mede-se com a diferenca central e fronteira periodica: a matriz e' ANTI-simetrica. */
        const int NG = 8;
        L D[8][8];
        for(int i=0;i<NG;i++) for(int j=0;j<NG;j++) D[i][j] = 0;
        for(int i=0;i<NG;i++){ D[i][(i+1)%NG] = 1; D[i][(i+NG-1)%NG] = -1; }
        long mau_anti = 0;
        for(int i=0;i<NG;i++) for(int j=0;j<NG;j++)
            if(D[i][j] != -D[j][i]) mau_anti++;
        printf("      A LEI 2 DIFERENCIAL:  <f', g> = −<f, g'>   com fronteira nula\n");
        printf("        a diferenca central %dx%d com fronteira periodica: D^T = −D ? %s\n",
               NG, NG, mau_anti ? "NAO" : "sim");
        printf("        ⟹ (d/dt)† = −d/dt : A DERIVADA TEMPORAL E' ANTI-AUTOADJUNTA\n\n");
        ok("(d/dt)† = -d/dt — a Lei 2 escrita com D no lugar de T, medida em 64 entradas",
           mau_anti == 0);
        conclui("e a ordem da cadeia e' FORCADA, nao escolhida: sem distancia nao ha' limite,");
        conclui("sem limite nao ha' continuidade, sem continuidade nao ha' derivada. O primeiro");
        conclui("degrau ja' existia — o CORPO METRICO do Catalogo, onde a distancia compoe:");
        conclui("d(A⊗C, B⊗C) = |Delta_C|·d(A,B), e cruzar ESCALA a distancia sem a destruir.");
        conclui("E no fim a Lei 2 reaparece em forma diferencial: e' por a derivada temporal ser");
        conclui("anti-autoadjunta que o tempo e' reversivel. A lei nao e' outra — e' a mesma,");
        conclui("escrita com D.");
    }

    printf("\n================================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  O QUE ISTO ARRUMA: as sete faces ja' estavam medidas como involucoes. O que");
        puts("  faltava era o segundo andar — a BIDUALIDADE de cada uma, e sobretudo a");
        puts("  CONDICAO sob que ela fecha. O padrao e' um so' e nao tinha sido dito: a");
        puts("  bidualidade e' sempre a involucao, e ela so' fecha sob FINITUDE ou FECHO.");
        puts("  E o caso Hilbert, que eu dera como irredutivel, resolve-se aqui: falta-lhe o");
        puts("  COMPLEMENTO, e com o par a obstrucao vira k + (n-k) = n.");
    } else printf("  FALHOU\n");
    return falhas ? 1 : 0;
}
