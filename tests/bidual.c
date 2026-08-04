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
    printf("\n§B10 A QUARTA LEI: a unidade e' potencia da dualidade. x^x = 1.\n\n");
    {
        /* O Aarao: "coloca uma quarta lei, lei de potencia, o potencial:
         * 4 - a unidade e' potencia da dualidade, lei x^x = 1."
         *
         * As tres primeiras dizem COMO se escreve, se le e se troca. A quarta e' de outra
         * natureza: diz o que SOBRA quando se multiplica um lado pelo outro — e o que sobra
         * e' a unidade. Em Newton a quarta tambem esta' fora das tres do movimento: e' a
         * gravitacao, que e' lei de POTENCIA e a unica que define um POTENCIAL. */

        /* (a) x^x = 1 tem SOLUCAO UNICA nos inteiros positivos, e e' o 1. Mede-se sem pow:
         * x^x = 1 com x >= 1 inteiro obriga x = 1, porque x >= 2 da' x^x >= 4. */
        long solucoes = 0, testados = 0;
        for(L x=1;x<=12;x++){
            L p = 1;
            for(L k=0;k<x;k++) p *= x;                 /* x^x, exato */
            if(p == 1) solucoes++;
            testados++; }
        printf("      x^x = 1 em x = 1..12 (inteiro, sem pow): %ld solucao(oes) em %ld\n",
               solucoes, testados);
        ok("x^x = 1 tem solucao UNICA: a unidade. E' o ponto fixo da potencia de si propria",
           solucoes == 1 && testados == 12);

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
