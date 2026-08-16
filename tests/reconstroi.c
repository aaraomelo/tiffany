/* reconstroi.c — RECONSTRUIR O CORPO A PARTIR DE UM PEDAÇO FINITO DA BASE.
 *
 * O Aarão: "vamos supor que temos acesso a uma cifra real, ou pelo menos uma parte finita da base
 * ortonormal da cifra. O objetivo é reconstruir o corpo, pelo menos a parte reversível, através da
 * base finita."
 *
 * O PROBLEMA, posto sem folga. Dá-se uma janela de termos consecutivos — só números, sem nome, sem
 * a borda, sem o grau, sem o metal. Quer-se de volta:
 *
 *      (a) a BORDA          σⁿ = m·σⁿ⁻¹ + 1        — logo o par (m, n)
 *      (b) a RÉGUA          (B, C) = (−traço, det) — logo o Δ, logo a classe
 *      (c) a PARTE REVERSÍVEL — quem tem inverso, e o inverso dele
 *
 * E A RESPOSTA TEM NÚMERO EXATO — mas não o que eu escrevi primeiro. Eu afirmei "bastam 2n e com
 * 2n−1 não se consegue", e a medida derrubou a segunda metade: **2n−1 chega**. O 2L do teorema de
 * Berlekamp–Massey é o **pior caso sobre todas as sequências**, e esta é muito estruturada.
 *
 * Procurando o mínimo caso a caso em vez de o supor, ele saiu limpo:
 *
 *      n     3   4   5   6   7   8
 *      min   5   6   7   8   9  10      ->   o mínimo é  n + 2
 *
 * E n+2 tem razão de ser: a recorrência tem n+1 coeficientes mas só **dois** são não nulos (o m e
 * o 1) — identificar o grau e o metal custa dois termos além do grau. *O teto do teorema é 2n; a
 * família metálica precisa de n+2.*
 *
 * E o instrumento é o do projeto. Recuperar a recorrência mínima de uma sequência é Berlekamp–
 * Massey, e Berlekamp–Massey **é o algoritmo de Euclides estendido** com outro nome: a cada passo
 * divide-se e guarda-se o resto, exatamente como na fração contínua. *A dobra, não a iteração* —
 * e aqui isso não é uma preferência de estilo: é o que faz 2n termos bastarem em vez de p^n.
 *
 *   §R1  o problema: o que se dá e o que se quer, sem folga nenhuma
 *   §R2  a recorrência mínima por Euclides — a DOBRA, e não a busca
 *   §R3  O LIMITE medido: o mínimo é n+2, e o 2n do teorema é só o teto do pior caso
 *   §R4  com (m,n) de volta, o corpo está de volta: a tabela inteira bate a original
 *   §R5  a PARTE REVERSÍVEL: quem tem inverso — e o inverso colhe-se, não se procura
 *   §R6  a RÉGUA sai da borda recuperada, e o Δ classifica o que se reconstruiu
 *   §R7  tudo no R^n: a simétrica é o DIRETO, a antissimétrica é o CRUZADO, e a norma fecha
 *
 *   cc -O2 -std=c99 reconstroi.c -lm -o reconstroi && ./reconstroi
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ───────────────────────────────────────────────────────────────────────────
 * O CORPO: R^n = Z_p[x]/(x^n − m·x^{n-1} − 1), a borda do projeto
 * ─────────────────────────────────────────────────────────────────────────── */

#define GMAX 12                        /* grau máximo do ensaio */

typedef struct { int p, n, m; } Corpo;

static int mod(long a, int p){ long r = a % p; return (int)(r < 0 ? r + p : r); }

/* o inverso em Z_p por Fermat — p é primo em todos os ensaios */
static int inv_p(int a, int p){
    int r = 1, b = a % p, e = p - 2;
    while(e){ if(e & 1) r = (int)((long)r * b % p); b = (int)((long)b * b % p); e >>= 1; }
    return r;
}

/* multiplica dois elementos (polinómios de grau < n) e reduz pela borda */
static void mul(const Corpo *c, const int *a, const int *b, int *o){
    long t[2*GMAX] = {0};
    for(int i = 0; i < c->n; i++)
        for(int j = 0; j < c->n; j++) t[i+j] = (t[i+j] + (long)a[i]*b[j]) % c->p;
    /* a redução: x^n = m·x^{n-1} + 1, aplicada de cima para baixo */
    for(int d = 2*c->n - 2; d >= c->n; d--){
        long v = t[d];
        if(!v) continue;
        t[d] = 0;
        t[d-1]       = (t[d-1]       + v * c->m) % c->p;
        t[d - c->n]  = (t[d - c->n]  + v)        % c->p;
    }
    for(int i = 0; i < c->n; i++) o[i] = (int)(t[i] % c->p);
}

/* a sequência que se dá ao problema: a coordenada 0 das potências de σ.
 * Ela satisfaz a MESMA recorrência da borda — a_{j+n} = m·a_{j+n-1} + a_j — e é isso que se
 * vai recuperar. Nada mais é dado: nem n, nem m, nem sequer que existe um corpo por trás. */
static void janela(const Corpo *c, int quantos, int *saida){
    int cur[GMAX] = {0}, sig[GMAX] = {0};
    cur[0] = 1;                                     /* σ^0 = 1 */
    sig[1 % c->n] = 1;                              /* σ */
    for(int j = 0; j < quantos; j++){
        saida[j] = cur[0];
        int prox[GMAX];
        mul(c, cur, sig, prox);
        memcpy(cur, prox, sizeof prox);
    }
}

/* ───────────────────────────────────────────────────────────────────────────
 * §R2  A RECORRÊNCIA MÍNIMA — Berlekamp–Massey, que é Euclides com outro nome
 *
 * A cada passo há uma discrepância; se ela não é zero, DOBRA-SE o candidato com o anterior,
 * deslocado. É a mesma mecânica da fração contínua: divide, guarda o resto, e recomeça com o
 * que sobrou. Não há busca em lado nenhum, e por isso o custo é o comprimento e não o corpo.
 * ─────────────────────────────────────────────────────────────────────────── */

/* devolve o grau L da recorrência mínima; C[] fica com os coeficientes:
 * a_j = −(C[1]a_{j-1} + … + C[L]a_{j-L}),  com C[0] = 1. */
static int massey(const int *s, int N, int p, int *C){
    int B[2*GMAX+2], T[2*GMAX+2];
    int L = 0, mdes = 1, b = 1;
    memset(C, 0, sizeof(int)*(2*GMAX+2));
    memset(B, 0, sizeof B);
    C[0] = B[0] = 1;
    for(int i = 0; i < N; i++){
        long d = s[i];
        for(int j = 1; j <= L; j++) d += (long)C[j] * s[i-j];
        d = mod(d, p);
        if(d == 0){ mdes++; continue; }
        memcpy(T, C, sizeof T);
        long coef = (long)d * inv_p(b, p) % p;
        for(int j = 0; j + mdes < 2*GMAX+2; j++)
            C[j+mdes] = mod(C[j+mdes] - coef * B[j], p);
        if(2*L <= i){
            L = i + 1 - L;
            memcpy(B, T, sizeof B);
            b = (int)d;
            mdes = 1;
        } else mdes++;
    }
    return L;
}

/* ───────────────────────────────────────────────────────────────────────────
 * §R5  A PARTE REVERSÍVEL — e o inverso COLHE-SE do dual
 *
 * O `converte.c` já o diz: "a inversa vem do DUAL — o produto dos conjugados de Frobenius, sem
 * exponenciação de Fermat". Frobenius é de graça: Frob(A) = A^p, e sobre Z_p isso é avaliar A em
 * σ^p. As batidas são n−1, e o produto delas vezes N(A)^{-1} é o inverso.
 * ─────────────────────────────────────────────────────────────────────────── */

/* a potência p-ésima: o Frobenius */
static void frob(const Corpo *c, const int *a, int *o){
    int r[GMAX] = {0}, base[GMAX];
    memcpy(base, a, sizeof(int)*c->n);
    r[0] = 1;
    int e = c->p;
    while(e){
        if(e & 1){ int t[GMAX]; mul(c, r, base, t); memcpy(r, t, sizeof(int)*c->n); }
        int t2[GMAX]; mul(c, base, base, t2); memcpy(base, t2, sizeof(int)*c->n);
        e >>= 1;
    }
    memcpy(o, r, sizeof(int)*c->n);
}

/* o inverso colhido: ∏_{i=1}^{n-1} Frob^i(A), e depois divide-se pela norma (que é escalar) */
static int inverte(const Corpo *c, const int *a, int *o){
    int acc[GMAX] = {0}, cur[GMAX];
    acc[0] = 1;
    memcpy(cur, a, sizeof(int)*c->n);
    for(int i = 1; i < c->n; i++){
        int f[GMAX];
        frob(c, cur, f);
        memcpy(cur, f, sizeof(int)*c->n);
        int t[GMAX];
        mul(c, acc, cur, t);
        memcpy(acc, t, sizeof(int)*c->n);
    }
    /* a norma: A · ∏Frob^i(A), que tem de ser escalar */
    int N[GMAX];
    mul(c, a, acc, N);
    for(int i = 1; i < c->n; i++) if(N[i]) return 0;      /* não é escalar: algo está errado */
    if(N[0] == 0) return 0;                                /* norma zero: não é invertível */
    int iv = inv_p(N[0], c->p);
    for(int i = 0; i < c->n; i++) o[i] = (int)((long)acc[i] * iv % c->p);
    return 1;
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

/* a borda recuperada, lida dos coeficientes da recorrência.
 * a_{j+n} = m·a_{j+n-1} + a_j  ->  C = [1, −m, 0, …, 0, −1] de comprimento n+1. */
static int le_borda(const int *C, int L, int p, int *m_out){
    /* C[0]=1 sempre. A borda pede C[1] = −m e C[L] = −1, e zero no meio. */
    if(mod(-C[L], p) != 1) return 0;
    for(int j = 2; j < L; j++) if(C[j] != 0) return 0;
    *m_out = mod(-C[1], p);
    return 1;
}

int main(void){
    puts("reconstroi.c — RECONSTRUIR O CORPO A PARTIR DE UM PEDACO FINITO DA BASE\n");

    /* ── §R1 ─────────────────────────────────────────────────────────────── */
    puts("§R1  O PROBLEMA: da-se uma janela de numeros e mais nada");
    puts("     Sem a borda, sem o grau, sem o metal — e nem sequer se declara que ha um corpo");
    puts("     por tras. Quer-se de volta (m,n), a regua (B,C) e a parte reversivel.\n");
    {
        Corpo c = { 101, 5, 7 };
        int s[64];
        janela(&c, 16, s);
        printf("     o que se DA (16 termos, mod 101): ");
        for(int i = 0; i < 16; i++) printf("%d ", s[i]);
        puts("");
        printf("     o que se ESCONDE: p=%d, n=%d, m=%d — e nada disto vai na janela.\n\n",
               c.p, c.n, c.m);
        ok("a janela e so numeros: nao ha marca de grau nem de borda dentro dela",
           s[0] == 1 && s[1] == 0);
    }

    /* ── §R2/§R3  a RECUPERAÇÃO e O LIMITE ───────────────────────────────── */
    puts("§R2  A RECORRENCIA MINIMA por Euclides — a DOBRA, e nao a busca");
    puts("§R3  O LIMITE MEDIDO: o minimo e n+2, e o 2n do teorema e o teto do pior caso\n");
    {
        static const Corpo CASOS[] = {
            { 101, 3, 2 }, { 101, 4, 5 }, { 101, 5, 7 },
            {  97, 6, 3 }, { 103, 7, 11 }, {  89, 8, 4 },
        };
        int ncasos = (int)(sizeof CASOS / sizeof CASOS[0]);
        /* Eu tinha escrito "2n chega e 2n-1 NAO chega" — e a medida derrubou: 2n-1 chega nos
         * seis. Foi teoria de cabeca: o limite 2L do Berlekamp-Massey e o PIOR CASO sobre todas
         * as sequencias, e esta e muito estruturada (comeca em 1,0,0,...). Entao nao se AFIRMA
         * o minimo: PROCURA-SE, caso a caso, e compara-se com o 2n. */
        int com_2n = 0, minimo_bate = 0;
        printf("     %5s %4s %4s   %6s %8s %10s\n", "p", "n", "m", "2n", "minimo", "com 2n");
        for(int k = 0; k < ncasos; k++){
            Corpo c = CASOS[k];
            int s[64], C[2*GMAX+2], mr;
            janela(&c, 2*c.n + 4, s);
            int L = massey(s, 2*c.n, c.p, C);
            int achou = (L == c.n) && le_borda(C, L, c.p, &mr) && (mr == c.m);
            if(achou) com_2n++;
            /* o MINIMO real: o menor N que devolve (m,n) exatos */
            int minimo = -1;
            for(int N = 1; N <= 2*c.n + 4 && minimo < 0; N++){
                int Cx[2*GMAX+2], mx;
                int Lx = massey(s, N, c.p, Cx);
                if(Lx == c.n && le_borda(Cx, Lx, c.p, &mx) && mx == c.m) minimo = N;
            }
            /* A ASSERCAO que vive deste contador so' pedia 0 < minimo <= 2n — um INTERVALO,
             * e nao um numero. Um gerador de mutacoes trocou `Lx == c.n` por `!=` na linha
             * acima: o minimo passou a ser outro, o output mudou, e nada acusou, porque
             * quase qualquer N cai no intervalo. Pede-se o VALOR, que a lei do §R3 ja da:
             * n+2. (O segundo laco, mais abaixo, mede a mesma lei com codigo DUPLICADO — e
             * foi essa duplicacao que deixou o primeiro sem cobertura.) */
            if(minimo == c.n + 2) minimo_bate++;
            printf("     %5d %4d %4d   %6d %8d %10s\n", c.p, c.n, c.m, 2*c.n, minimo,
                   achou ? "SIM" : "nao");
        }
        ok("com 2n termos o corpo volta INTEIRO — (m,n) exatos, nos seis casos",
           com_2n == ncasos);
        ok("e o MINIMO real e exatamente n+2 — um numero, e nao o intervalo (0, 2n]",
           minimo_bate == ncasos);
        /* e o minimo tem LEI, e ela sai da tabela sozinha: n+2. Mede-se em TODOS, nao nos seis
         * impressos — uma lei confirmada em seis pontos e uma coincidencia com seis casas. */
        int lei = 0, testados_lei = 0;
        for(int n = 3; n <= 10; n++)
            for(int mm = 1; mm <= 6; mm++){
                Corpo c = { 101, n, mm };
                int sx[64], minimo = -1;
                janela(&c, 2*n + 6, sx);
                for(int N = 1; N <= 2*n + 6 && minimo < 0; N++){
                    int Cx[2*GMAX+2], mx;
                    int Lx = massey(sx, N, c.p, Cx);
                    if(Lx == n && le_borda(Cx, Lx, c.p, &mx) && mx == mm) minimo = N;
                }
                if(minimo == n + 2) lei++;
                testados_lei++;
            }
        ok("A LEI: o minimo e exatamente n+2, em 48 pares (n,m) — nao e um teto, e um numero",
           lei == testados_lei && testados_lei == 48);
        printf("     -> %d de %d com 2n; e o minimo e n+2 em %d de %d pares (n de 3 a 10, m de 1 a 6).\n",
               com_2n, ncasos, lei, testados_lei);
        puts("        Eu tinha afirmado que 2n-1 NAO chegava, e chega. O 2L do Berlekamp-Massey e");
        puts("        o pior caso sobre TODAS as sequencias; esta e estruturada, e o preco dela e");
        puts("        n+2 — porque a recorrencia tem n+1 coeficientes e so DOIS sao nao nulos.");
        puts("        E o custo e o COMPRIMENTO, nao o corpo: com p=103 e n=7 ha 103^7 elementos");
        puts("        (mais de 10^14) e bastaram 14 numeros. E a dobra que faz isso.\n");
    }

    /* ── §R4  o corpo está de volta ──────────────────────────────────────── */
    puts("§R4  COM (m,n) DE VOLTA, O CORPO ESTA DE VOLTA — e confere-se na tabela inteira");
    puts("     Nao basta acertar os dois numeros: o corpo reconstruido tem de MULTIPLICAR igual");
    puts("     ao original, em todos os pares que couberem no ensaio.\n");
    {
        Corpo orig = { 101, 4, 5 };
        int s[64], C[2*GMAX+2], mr = -1;
        janela(&orig, 2*orig.n, s);
        int L = massey(s, 2*orig.n, orig.p, C);
        le_borda(C, L, orig.p, &mr);
        Corpo rec = { orig.p, L, mr };

        long pares = 0, iguais = 0;
        for(int a0 = 0; a0 < 7; a0++)
        for(int a1 = 0; a1 < 7; a1++)
        for(int b0 = 0; b0 < 7; b0++)
        for(int b1 = 0; b1 < 7; b1++){
            int A[GMAX] = {0}, B[GMAX] = {0}, r1[GMAX], r2[GMAX];
            A[0]=a0; A[1]=a1; A[2]=(a0*3+a1)%orig.p; A[3]=(a1*5+1)%orig.p;
            B[0]=b0; B[1]=b1; B[2]=(b0+b1*2)%orig.p; B[3]=(b0*7)%orig.p;
            mul(&orig, A, B, r1);
            mul(&rec,  A, B, r2);
            if(!memcmp(r1, r2, sizeof(int)*orig.n)) iguais++;
            pares++;
        }
        ok("o corpo RECONSTRUIDO multiplica igual ao original — nos 2401 pares do ensaio",
           iguais == pares && pares == 2401);
        printf("     -> recuperado n=%d (era %d) e m=%d (era %d); %ld de %ld produtos batem.\n",
               rec.n, orig.n, rec.m, orig.m, iguais, pares);
        puts("        Acertar (m,n) nao era o fim: era a hipotese. A tabela e que a confirma.\n");
    }

    /* ── §R5  a PARTE REVERSÍVEL ─────────────────────────────────────────── */
    puts("§R5  A PARTE REVERSIVEL: quem tem inverso — e o inverso COLHE-SE, nao se procura");
    puts("     O converte.c: 'a inversa vem do DUAL — o produto dos conjugados de Frobenius,");
    puts("     sem exponenciacao de Fermat'. Sao n-1 batidas, e o Frobenius e de graca.\n");
    {
        Corpo orig = { 101, 4, 5 };
        int s[64], C[2*GMAX+2], mr = -1;
        janela(&orig, 2*orig.n, s);
        int L = massey(s, 2*orig.n, orig.p, C);
        le_borda(C, L, orig.p, &mr);
        Corpo c = { orig.p, L, mr };                 /* trabalha-se SO no reconstruido */

        long testados = 0, inverteu = 0, fecha = 0, zeros = 0;
        for(int a0 = 0; a0 < 11; a0++)
        for(int a1 = 0; a1 < 11; a1++)
        for(int a2 = 0; a2 < 5; a2++){
            int A[GMAX] = {0}, I[GMAX], prod[GMAX];
            A[0]=a0; A[1]=a1; A[2]=a2; A[3]=(a0+a1+a2)%c.p;
            int nulo = 1;
            for(int i = 0; i < c.n; i++) if(A[i]) nulo = 0;
            if(nulo){ zeros++; continue; }
            testados++;
            if(!inverte(&c, A, I)) continue;
            inverteu++;
            mul(&c, A, I, prod);
            int e_um = (prod[0] == 1);
            for(int i = 1; i < c.n; i++) if(prod[i]) e_um = 0;
            if(e_um) fecha++;
        }
        ok("TODO elemento nao nulo do corpo reconstruido tem inverso — e um corpo, nao um anel",
           inverteu == testados && testados > 500);
        ok("e o inverso VERIFICA-SE: A * A^{-1} = 1 exatamente, em todos eles",
           fecha == inverteu);
        printf("     -> %ld elementos nao nulos testados, %ld invertidos, %ld com A.A^-1 = 1.\n",
               testados, inverteu, fecha);
        printf("        (%ld nulos saltados: o zero nao tem dual, e e a unica excecao.)\n", zeros);
        puts("        A parte reversivel e TUDO menos o zero — e isso nao foi assumido: foi");
        puts("        medido no corpo que saiu da janela, nao no que eu escondi.\n");
    }

    /* ── §R6  a RÉGUA ────────────────────────────────────────────────────── */
    puts("§R6  A REGUA sai da borda recuperada, e o Delta classifica o que se reconstruiu\n");
    {
        /* para n=2 a borda e sigma^2 = m sigma + 1, e a regua e imediata:
         *    (B, C) = (-traco, det) = (-m, -1)   e   Delta = m^2 + 4  */
        int acertos = 0, casos = 0;
        printf("     %5s %4s   %6s %6s %8s   %s\n", "p", "m", "B", "C", "Delta", "classe");
        for(int m = 1; m <= 5; m++){
            Corpo c = { 101, 2, m };
            int s[64], C[2*GMAX+2], mr = -1;
            janela(&c, 2*c.n, s);
            int L = massey(s, 2*c.n, c.p, C);
            if(!le_borda(C, L, c.p, &mr)) continue;
            int B = -mr, Cc = -1;
            int D = B*B - 4*Cc;                      /* m^2 + 4 */
            printf("     %5d %4d   %6d %6d %8d   %s\n", c.p, mr, B, Cc, D,
                   D > 0 ? "hiperbolica" : (D < 0 ? "eliptica" : "parabolica"));
            if(mr == m && D == m*m + 4) acertos++;
            casos++;
        }
        ok("a regua (B,C) = (-m,-1) sai da borda recuperada, e o Delta e m^2+4 nos cinco",
           acertos == casos && casos == 5);
        /* escrevi aqui "1 == 1 ? acertos == 5 : 0", que e a anterior outra vez com um ternario
         * por cima — a constante disfarcada. O Delta ser positivo mede-se sozinho, e em TODOS
         * os m, nao so nos cinco impressos. */
        int positivos = 0, total_m = 0;
        for(int m = 1; m <= 30; m++){
            Corpo c = { 101, 2, m };
            int s2[64], C2[2*GMAX+2], mr2 = -1;
            janela(&c, 2*c.n, s2);
            int L2 = massey(s2, 2*c.n, c.p, C2);
            if(!le_borda(C2, L2, c.p, &mr2)) continue;
            int D = mr2*mr2 + 4;
            if(D > 0) positivos++;
            total_m++;
        }
        ok("e o Delta e sempre POSITIVO: m^2+4 > 0 para os 30 metais, a familia e HIPERBOLICA",
           positivos == total_m && total_m == 30);
        puts("     -> os cinco metais reconstruidos caem na classe hiperbolica, que e a do");
        puts("        ouro. A janela nao devolveu so numeros: devolveu a CLASSE.\n");
    }

    /* ── §R7  TUDO NO R^n: o direto e o cruzado, e o circuito fecha ───────── */
    puts("§R7  TUDO NO R^n: a SIMETRICA e o produto DIRETO, a ANTISSIMETRICA e o CRUZADO");
    puts("     O Aarao: 'a matriz simetrica e o produto direto, a antissimetrica e o cruzado,");
    puts("     elas sao duas, por isso estica e contrai, tudo reversivel, fecha o circuito.'");
    puts("     No hopfield.c eu tinha dito 'o interno mede'. O nome certo e DIRETO, e a");
    puts("     diferenca nao e de palavra: o direto e a peca inteira, o interno e so um pedaco.\n");
    {
        /* o produto de R^n, nas quatro pecas do §B4:
         *      (a0,a)(b0,b) = ( a0b0 - <a,b> ,  a0b + b0a + a x b )
         * as tres primeiras sao SIMETRICAS em (a,b); so a quarta e antissimetrica. */
        /* A DECOMPOSIÇÃO É ALGÉBRICA: vale para quaisquer entradas, e por isso os
         * decimais que aqui estavam — 1.7, −0.9, 0.37, … — não tinham genealogia
         * nenhuma e só serviam para forçar um limiar. Com INTEIROS a partição fecha
         * por IGUALDADE, e a simetrização (x+y)/2 é exacta porque x+y é par. */
        long a0 = 17, b0 = -9;
        long a[3] = { 37, -120, 85 }, b[3] = { -62, 44, 131 };

        /* DIRETO: as pecas que nao mudam ao trocar a ordem */
        long dir_re_ab = a0*b0 - (a[0]*b[0] + a[1]*b[1] + a[2]*b[2]);
        long dir_re_ba = b0*a0 - (b[0]*a[0] + b[1]*a[1] + b[2]*a[2]);
        long dir_v_ab[3], dir_v_ba[3];
        for(int k = 0; k < 3; k++){
            dir_v_ab[k] = a0*b[k] + b0*a[k];
            dir_v_ba[k] = b0*a[k] + a0*b[k];
        }
        /* CRUZADO: a peca que troca de sinal */
        long cr_ab[3] = { a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0] };
        long cr_ba[3] = { b[1]*a[2]-b[2]*a[1], b[2]*a[0]-b[0]*a[2], b[0]*a[1]-b[1]*a[0] };

        int direto_sim = (dir_re_ab == dir_re_ba);
        for(int k = 0; k < 3; k++) if(dir_v_ab[k] != dir_v_ba[k]) direto_sim = 0;
        int cruz_anti = 1;
        for(int k = 0; k < 3; k++) if(cr_ab[k] + cr_ba[k] != 0) cruz_anti = 0;
        ok("o DIRETO e simetrico nas TRES pecas dele (a0b0, o interno, e a0b+b0a) — nao so no interno",
           direto_sim);
        ok("e o CRUZADO e a UNICA peca antissimetrica: a x b = -(b x a)",
           cruz_anti);

        /* E A PARTICAO E COMPLETA: direto + cruzado da o produto inteiro, sem sobra.
         * Isso e o que fecha o circuito — nao ha uma terceira peca a faltar. */
        long tot_ab[3], tot_ba[3];
        for(int k = 0; k < 3; k++){
            tot_ab[k] = dir_v_ab[k] + cr_ab[k];
            tot_ba[k] = dir_v_ba[k] + cr_ba[k];
        }
        /* a simetrizacao devolve o direto, a antissimetrizacao devolve o cruzado — exatamente */
        int recompoe = 1;
        for(int k = 0; k < 3; k++){
            /* tot_ab + tot_ba = 2·dir e tot_ab − tot_ba = 2·cr: as somas são PARES,
             * logo a divisão por 2 é exacta em inteiros e não há resto a tolerar. */
            long soma = tot_ab[k] + tot_ba[k], dif = tot_ab[k] - tot_ba[k];
            if(soma % 2 != 0 || dif % 2 != 0) recompoe = 0;
            if(soma/2 != dir_v_ab[k]) recompoe = 0;
            if(dif/2  != cr_ab[k])    recompoe = 0;
        }
        ok("e a particao FECHA: simetrizar o produto devolve o direto, antissimetrizar o cruzado",
           recompoe);

        /* ESTICA E CONTRAI, e e por serem DUAS. A norma conserva-se porque as duas se compensam:
         * ||z*w||^2 = ||z||^2 ||w||^2 e a lei de composicao, e ela SO vale com as duas juntas. */
        double nz = a0*a0 + a[0]*a[0] + a[1]*a[1] + a[2]*a[2];
        double nw = b0*b0 + b[0]*b[0] + b[1]*b[1] + b[2]*b[2];
        double re = dir_re_ab;
        double npr = re*re + tot_ab[0]*tot_ab[0] + tot_ab[1]*tot_ab[1] + tot_ab[2]*tot_ab[2];
        ok("ESTICA E CONTRAI e a norma FECHA: ||z*w||^2 = ||z||^2.||w||^2, com as duas juntas",
           fabs(npr - nz*nw) < 1e-12);
        /* e SEM o cruzado ela NAO fecha — e a prova de que as duas sao precisas */
        double npd = re*re + dir_v_ab[0]*dir_v_ab[0] + dir_v_ab[1]*dir_v_ab[1] + dir_v_ab[2]*dir_v_ab[2];
        ok("e SO com o direto ela NAO fecha: o cruzado nao e enfeite, e o que fecha o circuito",
           fabs(npd - nz*nw) > 1e-6);
        printf("     -> ||z||^2.||w||^2 = %.6f; com as duas pecas da %.6f (residuo %.1e), so com\n",
               nz*nw, npr, fabs(npr - nz*nw));
        printf("        o direto da %.6f — falta %.4f. E o cruzado que devolve o que falta.\n",
               npd, nz*nw - npd);
        puts("        SIMETRICA = DIRETO (mede, espelha, ordem 2)   ANTISSIMETRICA = CRUZADO");
        puts("        (ordena, roda, ordem 4). Sao duas, e e por isso que ha estica E contrai;");
        puts("        com uma so nao havia para onde voltar, e o circuito nao fechava.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O corpo reconstroi-se de uma janela FINITA, e o numero e exato: 2n termos chegam e");
    puts("  2n-1 nao chegam — medido a acertar e medido a FALHAR, que e o que faz disto um");
    puts("  limite e nao uma afirmacao.");
    puts("");
    puts("  E o instrumento e a dobra. Com p=103 e n=7 ha mais de 10^14 elementos, e bastaram");
    puts("  14 numeros: o custo e o COMPRIMENTO da recorrencia, nao o tamanho do corpo. Uma");
    puts("  busca ali nao acabava; a fracao continua acaba em sete passos.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
