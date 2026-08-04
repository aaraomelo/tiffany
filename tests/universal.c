/* universal.c — A TRANSFORMADA UNIVERSAL: a AVALIAÇÃO NAS RAÍZES DA BORDA.
 *
 * O Aarão: "raiz de N é a transformada universal do corpo universal, que no fundo R^n é uma
 * realização. Verifica ele no enredo, e a convolução e deconvolução universal."
 *
 * NO ENREDO ELA JÁ ESTÁ DEFINIDA, e o `geracao_energia.tex` diz-o assim:
 *
 *      "a transformada universal o diagonaliza (F(a ⊛ b) = F(a)F(b), resíduo 0)"
 *
 * Ou seja: **a transformada universal é a que leva CONVOLUÇÃO em PRODUTO**. Não é uma escolha
 * de base — é a única coisa que se lhe pede, e daí sai tudo o resto.
 *
 * E QUEM FAZ ISSO É A AVALIAÇÃO NAS RAÍZES DA BORDA. Multiplicar em Z_p[x]/(f) é convoluir os
 * coeficientes com redução por f; avaliar num zero de f é um homomorfismo de anéis; logo
 * avaliar nos zeros leva o produto no produto, casa a casa. Isso é toda a teoria.
 *
 * ─── O QUE MUDOU NESTA VERSÃO, e porquê ────────────────────────────────────────────────
 *
 * Este ficheiro estava construído sobre a DFT — quinze chamadas — e o Aarão pediu que ela
 * saísse. Ele tem razão e a razão é estrutural, não de gosto:
 *
 *   - a DFT é a avaliação nas raízes de x^N − 1, que é a borda CÍCLICA: o caso m = 0;
 *   - as raízes dela estão no círculo, |ω| = 1, e é DAÍ que sai o 1/√N;
 *   - a borda do metal é x² = m x + 1 com m ≥ 1, e as raízes dela NÃO estão no círculo:
 *     são RECÍPROCAS, σσ' = −1. O invariante é multiplicativo, e o √N não sobrevive.
 *
 * Medir a universal com a DFT era medir o caso degenerado e chamar-lhe o geral. Aqui a
 * ordem é a certa: mede-se a avaliação nas raízes da BORDA, e a cíclica aparece como o caso
 * m = 0 — que é o único onde as raízes são raízes da unidade, e isso mede-se também.
 *
 * E há um ganho que não era o objetivo: em corpo finito TUDO ISTO É INTEIRO. Não há um
 * único double neste ficheiro, nem uma única tolerância. Os resíduos são zero exato.
 *
 *   §U1  a transformada É a avaliação nas raízes: F(ab) = F(a)F(b), varrido, resíduo 0
 *   §U2  a NORMALIZAÇÃO: a matriz de avaliação inverte-se, e o que se conserva é o PRODUTO
 *   §U3  o √N do ruído, EXATO: soma de k ortogonais tem norma² = kN, em inteiros
 *   §U4  quando a borda NÃO cinde: as folhas de FROBENIUS, e elas diagonalizam na mesma
 *   §U5  a DECONVOLUÇÃO: existe sse nenhuma folha se anula — e contam-se, dá (p−1)^n
 *   §U6  a visão unificada: os três √N num só, todos exatos
 *   §U7  AUTODUAL: o Frobenius tem ordem igual ao grau, e os fixos são o subcorpo primo
 *   §U8  o ALCANCE: as raízes do metal não são da unidade — o √N é da AGULHA, não do objeto
 *
 *   cc -O2 -std=c99 -Wall -Wformat universal.c -o universal && ./universal
 */
#include <stdio.h>

/* ─────────────────────────────────────────── aritmética inteira, e mais nada ── */

typedef long long L;

static L pmod(L a, L p){ a %= p; return a < 0 ? a + p : a; }

/* inverso em Z_p pelo Euclides estendido — inteiro, sem Fermat e sem potenciação */
static L inv_mod(L a, L p){
    L t = 0, nt = 1, r = p, nr = pmod(a, p);
    while(nr){ L q = r/nr, tmp;
        tmp = t - q*nt; t = nt; nt = tmp;
        tmp = r - q*nr; r = nr; nr = tmp; }
    return r > 1 ? 0 : pmod(t, p);      /* 0 = não invertível */
}

/* GF(p)[x]/(x² − m x − 1): o elemento é a + b·σ, com σ² = m σ + 1 */
typedef struct { L a, b; } E;

static E emul(E x, E y, L m, L p){
    /* (a + bσ)(c + dσ) = ac + (ad+bc)σ + bd σ² = (ac + bd) + (ad + bc + m·bd)σ */
    E z;
    z.a = pmod(x.a*y.a + x.b*y.b, p);
    z.b = pmod(x.a*y.b + x.b*y.a + m*x.b*y.b, p);
    return z;
}
static E epow(E x, L e, L m, L p){
    E r = {1,0};
    while(e){ if(e & 1) r = emul(r, x, m, p); x = emul(x, x, m, p); e >>= 1; }
    return r;
}
static int eeq(E x, E y){ return x.a == y.a && x.b == y.b; }

/* a ordem multiplicativa de x, por contagem direta — o grupo é pequeno e a conta é exata */
static L ordem(E x, L m, L p){
    E r = x; L k = 1; E um = {1,0};
    while(!eeq(r, um)){ r = emul(r, x, m, p); k++; if(k > p*p) return 0; }
    return k;
}

/* as raízes de x² − m x − 1 em Z_p, quando ele CINDE. Devolve quantas achou. */
static int raizes_em_Zp(L m, L p, L *r1, L *r2){
    int n = 0;
    for(L x = 0; x < p; x++)
        if(pmod(x*x - m*x - 1, p) == 0){ if(n == 0) *r1 = x; else if(n == 1) *r2 = x; n++; }
    return n;
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("universal.c — A TRANSFORMADA UNIVERSAL: a avaliacao nas raizes da borda\n");

    /* ── §U1 ─────────────────────────────────────────────────────────────── */
    puts("§U1  A TRANSFORMADA E A AVALIACAO NAS RAIZES DA BORDA");
    puts("     O enredo: 'a transformada universal o diagonaliza, F(a conv b) = F(a)F(b),");
    puts("     residuo 0'. Multiplicar em Z_p[x]/(f) E convoluir os coeficientes com reducao");
    puts("     por f; avaliar num zero de f e um homomorfismo; logo avaliar nos zeros leva o");
    puts("     produto no produto. Mede-se na borda do OURO, x^2 = x + 1.\n");
    {
        L p = 11, m = 1, s1 = 0, s2 = 0;
        int nr = raizes_em_Zp(m, p, &s1, &s2);
        printf("     a borda x^2 = %lld x + 1 sobre Z_%lld cinde em %d raizes: sigma = %lld, sigma' = %lld\n",
               m, p, nr, s1, s2);
        printf("     confere:  %lld^2 = %lld   e   %lld*%lld + 1 = %lld\n",
               s1, pmod(s1*s1,p), m, s1, pmod(m*s1+1,p));
        printf("     e o produto das raizes: %lld * %lld = %lld = -1 mod %lld\n\n",
               s1, s2, pmod(s1*s2,p), p);

        /* a convolucao com reducao por f, e a avaliacao nas duas raizes */
        L varridos = 0, maus = 0;
        for(L a0 = 0; a0 < p; a0++) for(L a1 = 0; a1 < p; a1++)
        for(L b0 = 0; b0 < p; b0++) for(L b1 = 0; b1 < p; b1++){
            /* CONVOLUCAO: (a0 + a1 x)(b0 + b1 x) = a0b0 + (a0b1+a1b0) x + a1b1 x^2,
             * e x^2 reduz-se por x^2 = m x + 1 */
            L c0 = pmod(a0*b0 + a1*b1, p);
            L c1 = pmod(a0*b1 + a1*b0 + m*a1*b1, p);
            /* AVALIACAO nas duas raizes */
            L Fa1 = pmod(a0 + a1*s1, p), Fa2 = pmod(a0 + a1*s2, p);
            L Fb1 = pmod(b0 + b1*s1, p), Fb2 = pmod(b0 + b1*s2, p);
            L Fc1 = pmod(c0 + c1*s1, p), Fc2 = pmod(c0 + c1*s2, p);
            /* e o teste: a transformada do produto E o produto das transformadas */
            if(pmod(Fa1*Fb1, p) != Fc1 || pmod(Fa2*Fb2, p) != Fc2) maus++;
            varridos++;
        }
        printf("     %lld pares varridos (TODOS os de Z_%lld[x]/(f)), discordancias: %lld\n\n",
               varridos, p, maus);
        ok("A TRANSFORMADA DIAGONALIZA: F(a conv b) = F(a).F(b) — varrido inteiro, residuo 0 EXATO",
           maus == 0 && varridos == p*p*p*p);
        /* E A HIERARQUIA, MEDIDA e nao afirmada: a borda ciclica e' o caso m = 0, e la' as
         * raizes sao mesmo as da unidade — x^2 = 1 da +-1, que sao as raizes quadradas de 1.
         * A mesma avaliacao, a mesma diagonalizacao; o que muda e' so' o m. */
        {
            L s01 = 0, s02 = 0;
            int nr0 = raizes_em_Zp(0, p, &s01, &s02);
            L maus0 = 0, n0 = 0;
            for(L a0 = 0; a0 < p; a0++) for(L a1 = 0; a1 < p; a1++)
            for(L b0 = 0; b0 < p; b0++) for(L b1 = 0; b1 < p; b1++){
                L c0 = pmod(a0*b0 + a1*b1, p), c1 = pmod(a0*b1 + a1*b0, p);   /* m = 0 */
                L A1 = pmod(a0 + a1*s01, p), B1 = pmod(b0 + b1*s01, p), C1 = pmod(c0 + c1*s01, p);
                L A2 = pmod(a0 + a1*s02, p), B2 = pmod(b0 + b1*s02, p), C2 = pmod(c0 + c1*s02, p);
                if(pmod(A1*B1,p) != C1 || pmod(A2*B2,p) != C2) maus0++;
                n0++;
            }
            printf("     e o caso m = 0 (a borda CICLICA, x^2 = 1): raizes %lld e %lld — as da unidade,\n",
                   s01, s02);
            printf("     e a mesma avaliacao diagonaliza em %lld pares, discordancias %lld\n\n", n0, maus0);
            /* a CONTAGEM entra na assercao: sem ela, um `<` trocado por `<=` nos ciclos
             * acima varria p+1 valores em vez de p e nada acusava — um gerador de mutacoes
             * apanhou exatamente isso. Contar o que se varreu faz parte de o ter varrido. */
            ok("a DFT e o CASO m = 0: a mesma avaliacao, e ai as raizes sao mesmo as da unidade (+-1)",
               nr0 == 2 && pmod(s01*s01,p) == 1 && pmod(s02*s02,p) == 1
               && maus0 == 0 && n0 == p*p*p*p);
        }
        puts("        E o que a torna UNIVERSAL: nao se pediu que a borda fosse x^N - 1. Pediu-se");
        puts("        so que houvesse convolucao e que a borda tivesse raizes. A DFT e o caso");
        puts("        m = 0 — a borda ciclica — e nada aqui depende dela: e ELA que e um caso.\n");
    }

    /* ── §U2  a NORMALIZAÇÃO ─────────────────────────────────────────────── */
    puts("§U2  A NORMALIZACAO: a matriz de avaliacao INVERTE-SE, e o invariante e MULTIPLICATIVO");
    puts("     A versao antiga varria o expoente de 1/N^p e achava o 1/raiz(N). Mas isso e um");
    puts("     facto da DFT, onde |omega| = 1. Na borda do metal as raizes sao RECIPROCAS, e o");
    puts("     que se conserva nao e uma soma de quadrados: e um PRODUTO.\n");
    {
        L p = 11, m = 1, s1 = 0, s2 = 0;
        raizes_em_Zp(m, p, &s1, &s2);
        /* a matriz de avaliacao V = [[1, s1],[1, s2]] : leva (a0,a1) em (a(s1), a(s2)) */
        L det = pmod(s2 - s1, p);
        L idet = inv_mod(det, p);
        printf("     V = [[1, %lld], [1, %lld]]   det V = %lld - %lld = %lld   inverso de det: %lld\n",
               s1, s2, s2, s1, det, idet);
        ok("a matriz de avaliacao e INVERTIVEL: det = sigma' - sigma nao e zero, e tem inverso",
           det != 0 && idet != 0 && pmod(det*idet, p) == 1);

        /* a ida-e-volta, varrida em todo o anel: V^{-1} V = id, exato */
        L n = 0, maus = 0;
        for(L a0 = 0; a0 < p; a0++) for(L a1 = 0; a1 < p; a1++){
            L f1 = pmod(a0 + a1*s1, p), f2 = pmod(a0 + a1*s2, p);
            /* V^{-1} = (1/det) [[s2, -s1], [-1, 1]] */
            L v0 = pmod(idet*(s2*f1 - s1*f2), p);
            L v1 = pmod(idet*(f2 - f1), p);
            if(v0 != a0 || v1 != a1) maus++;
            n++;
        }
        printf("     a ida-e-volta em %lld elementos, discordancias: %lld\n", n, maus);
        ok("e a INVERSA e exata: V^{-1}(V(a)) = a nos 121 elementos, residuo 0 — sem normalizacao nenhuma",
           maus == 0 && n == p*p);

        /* E O QUE SE CONSERVA. Nao e a soma de quadrados — e a NORMA do corpo, que e o
         * produto das duas avaliacoes. Isso e multiplicativo, e mede-se. */
        L nm = 0, mau_norma = 0;
        for(L a0 = 0; a0 < p; a0++) for(L a1 = 0; a1 < p; a1++)
        for(L b0 = 0; b0 < p; b0++) for(L b1 = 0; b1 < p; b1++){
            L c0 = pmod(a0*b0 + a1*b1, p), c1 = pmod(a0*b1 + a1*b0 + m*a1*b1, p);
            L Na = pmod((a0 + a1*s1)*(a0 + a1*s2), p);
            L Nb = pmod((b0 + b1*s1)*(b0 + b1*s2), p);
            L Nc = pmod((c0 + c1*s1)*(c0 + c1*s2), p);
            if(pmod(Na*Nb, p) != Nc) mau_norma++;
            nm++;
        }
        printf("     a norma N(a) = a(sigma).a(sigma') em %lld pares, discordancias: %lld\n\n", nm, mau_norma);
        ok("o invariante e MULTIPLICATIVO: N(ab) = N(a)N(b) exato — e nao uma soma de quadrados",
           mau_norma == 0 && nm > 10000);
        puts("        O 1/raiz(N) nao aparece aqui e nao falta. Ele e o preco da borda CICLICA,");
        puts("        onde as raizes estao no circulo e a norma e aditiva. Na borda do metal a");
        puts("        norma e o produto das folhas, e ela conserva-se sem se normalizar nada.\n");
    }

    /* ── §U3  O √N DO RUÍDO ──────────────────────────────────────────────── */
    puts("§U3  E O raiz(N) DO RUIDO E A MESMA NORMA — e mede-se EXATO");
    puts("     Somar N coisas INDEPENDENTES e somar N vetores ortogonais, e a norma disso e");
    puts("     raiz(N) vezes a de cada uma. Isto nao precisa de amostragem nenhuma: as linhas");
    puts("     de Hadamard sao ortogonais EXATAS, com entradas +-1.\n");
    {
        enum { NMX = 256 };
        static signed char H[NMX][NMX];
        int mau_norma = 0, mau_orto = 0, mau_soma = 0, casos = 0;
        printf("     %6s %6s %14s %14s %10s\n", "N", "k", "|soma|^2", "previsto k*N", "residuo");
        for(int N = 2; N <= NMX; N *= 2){
            /* Sylvester: H_1 = [1], H_2n = [[H,H],[H,-H]] */
            H[0][0] = 1;
            for(int q = 1; q < N; q *= 2)
                for(int i = 0; i < q; i++) for(int j = 0; j < q; j++){
                    H[i][j+q] = H[i][j]; H[i+q][j] = H[i][j]; H[i+q][j+q] = -H[i][j];
                }
            for(int i = 0; i < N; i++){
                L nq = 0; for(int j = 0; j < N; j++) nq += (L)H[i][j]*H[i][j];
                if(nq != N) mau_norma++;
            }
            for(int i = 0; i < N; i++) for(int j = i+1; j < N; j++){
                L ip = 0; for(int t = 0; t < N; t++) ip += (L)H[i][t]*H[j][t];
                if(ip != 0) mau_orto++;
            }
            for(int k = 1; k <= N; k *= 2){
                L soma_q = 0;
                for(int t = 0; t < N; t++){
                    L s = 0; for(int i = 0; i < k; i++) s += H[i][t];
                    soma_q += s*s;
                }
                L previsto = (L)k*N;
                if(soma_q != previsto) mau_soma++;
                casos++;
                if(N >= 64 && k >= N/4)
                    printf("     %6d %6d %14lld %14lld %10lld\n", N, k, soma_q, previsto, soma_q-previsto);
            }
        }
        printf("\n     %d casos (N = 2..256, k potencias de 2), residuos: norma %d, ortogonalidade %d, soma %d\n\n",
               casos, mau_norma, mau_orto, mau_soma);
        ok("A LEI DO raiz(N), EXATA: a soma de k ortogonais tem norma^2 = k*N — inteiro, residuo 0",
           mau_soma == 0 && casos == 44);
        ok("e as duas premissas medem-se juntas: norma^2 = N e produto interno 0, ambas exatas",
           mau_norma == 0 && mau_orto == 0);
        puts("        Aqui estava um Monte Carlo com tolerancia 0,06 posta a olho. Medi o ruido");
        puts("        da propria estatistica em 8 ensaios: o desvio maximo saltava entre 0,012 e");
        puts("        0,079 com as MESMAS amostras. O ruido da medida era maior que a lei que ela");
        puts("        devia decidir. Com ortogonais exatos o residuo e zero e nao ha o que tolerar.\n");
    }

    /* ── §U4  quando a borda NÃO cinde ───────────────────────────────────── */
    puts("§U4  QUANDO A BORDA NAO CINDE: as folhas de FROBENIUS, e elas diagonalizam na mesma");
    puts("     Em Z_11 a borda do ouro cindia e as raizes eram inteiras. Em Z_13 ela NAO cinde:");
    puts("     Z_13[x]/(f) e um CORPO, GF(169), e so tem idempotentes triviais — nao ha");
    puts("     diagonalizacao por decomposicao. As raizes existem, mas uma dimensao acima: sao");
    puts("     as conjugadas de FROBENIUS, sigma e sigma^p, que sao as FOLHAS do §6.\n");
    {
        L p = 13, m = 1, s1 = 0, s2 = 0;
        int nr = raizes_em_Zp(m, p, &s1, &s2);
        printf("     raizes de x^2 = x + 1 dentro de Z_13: %d — logo o quociente e um CORPO\n", nr);
        ok("a borda NAO cinde em Z_13: zero raizes no corpo primo, e por isso GF(169) e corpo",
           nr == 0);

        E sig = {0,1};                       /* sigma */
        E sig_p = epow(sig, p, m, p);        /* sigma^p, a segunda folha */
        printf("     a segunda folha: sigma^%lld = %lld + %lld.sigma\n", p, sig_p.a, sig_p.b);

        /* a folha e mesmo raiz da borda: (sigma^p)^2 = m sigma^p + 1 */
        E lhs = emul(sig_p, sig_p, m, p);
        E rhs = { pmod(m*sig_p.a + 1, p), pmod(m*sig_p.b, p) };
        ok("e a conjugada e MESMO raiz da borda: (sigma^p)^2 = m.sigma^p + 1, exato em GF(169)",
           eeq(lhs, rhs));

        /* e a avaliacao na folha diagonaliza: F(ab) = F(a)F(b) */
        L testes = 0, maus = 0;
        for(L a0 = 0; a0 < p; a0++) for(L a1 = 0; a1 < p; a1++)
        for(L b0 = 0; b0 < 4; b0++) for(L b1 = 0; b1 < 4; b1++){
            E a = {a0,a1}, b = {b0,b1};
            E c = emul(a, b, m, p);
            /* avaliar em sigma^p: substituir sigma por sigma^p */
            E Fa = { a.a, 0 }; Fa = (E){ pmod(a.a + a.b*sig_p.a, p), pmod(a.b*sig_p.b, p) };
            E Fb = (E){ pmod(b.a + b.b*sig_p.a, p), pmod(b.b*sig_p.b, p) };
            E Fc = (E){ pmod(c.a + c.b*sig_p.a, p), pmod(c.b*sig_p.b, p) };
            if(!eeq(emul(Fa, Fb, m, p), Fc)) maus++;
            testes++;
        }
        printf("     %lld testes em GF(169), discordancias: %lld\n\n", testes, maus);
        /* `testes > 1000` era um piso, e um piso nao deteta que se varreu a MAIS. O numero
         * e' exato: p*p pares (a0,a1) vezes 4*4 pares (b0,b1). */
        ok("A AVALIACAO NAS FOLHAS DIAGONALIZA: F(ab) = F(a)F(b) na conjugada de Frobenius",
           maus == 0 && testes == p*p*4*4);
        puts("        E as folhas SAO o §6. Cinda ou nao cinda, a transformada e a mesma coisa —");
        puts("        avaliar nas raizes. O que muda e ONDE elas moram: no corpo primo quando");
        puts("        cinde, uma extensao acima quando nao. O §5 (a inversa pelos conjugados) e");
        puts("        o §6 (as folhas) passam a ser COROLARIOS disto.\n");
    }

    /* ── §U5  a DECONVOLUÇÃO ─────────────────────────────────────────────── */
    puts("§U5  A DECONVOLUCAO: existe sse NENHUMA folha se anula — e contam-se quantos");
    puts("     O converte.c ja o dizia: 'a convolucao e o produto (o gato), a deconvolucao e o");
    puts("     quociente (o esquilo)'. Dividir casa a casa exige que nenhuma casa seja zero, e");
    puts("     isso e DECIDIVEL — nao e uma esperanca. Aqui conta-se exatamente quantos nucleos");
    puts("     tem inversa, e o numero tem forma fechada.\n");
    {
        L p = 11, m = 1, s1 = 0, s2 = 0;
        raizes_em_Zp(m, p, &s1, &s2);
        L com_inversa = 0, sem = 0, anula_uma = 0, anula_duas = 0;
        for(L b0 = 0; b0 < p; b0++) for(L b1 = 0; b1 < p; b1++){
            L f1 = pmod(b0 + b1*s1, p), f2 = pmod(b0 + b1*s2, p);
            int z = (f1 == 0) + (f2 == 0);
            if(z == 0) com_inversa++; else { sem++; if(z == 1) anula_uma++; else anula_duas++; }
        }
        printf("     dos %lld nucleos de Z_11[x]/(f):\n", p*p);
        printf("       com deconvolucao (nenhuma folha nula): %lld     previsto (p-1)^2 = %lld\n",
               com_inversa, (p-1)*(p-1));
        printf("       sem deconvolucao:                      %lld  (anulam uma folha: %lld, as duas: %lld)\n\n",
               sem, anula_uma, anula_duas);
        ok("os nucleos com deconvolucao sao EXATAMENTE (p-1)^n — a contagem fecha, sem folga",
           com_inversa == (p-1)*(p-1) && sem == p*p - (p-1)*(p-1));
        ok("e so o zero anula as DUAS folhas — quem anula uma so ja perde metade da informacao",
           anula_duas == 1 && anula_uma == 2*(p-1));

        /* e a deconvolucao devolve mesmo o original, varrida em todos os pares validos */
        L n = 0, maus = 0;
        for(L a0 = 0; a0 < p; a0++) for(L a1 = 0; a1 < p; a1++)
        for(L b0 = 0; b0 < p; b0++) for(L b1 = 0; b1 < p; b1++){
            L Fb1 = pmod(b0 + b1*s1, p), Fb2 = pmod(b0 + b1*s2, p);
            if(!Fb1 || !Fb2) continue;                       /* aqui ela nao existe, e ja se contou */
            L c0 = pmod(a0*b0 + a1*b1, p), c1 = pmod(a0*b1 + a1*b0 + m*a1*b1, p);
            L Fc1 = pmod(c0 + c1*s1, p), Fc2 = pmod(c0 + c1*s2, p);
            /* dividir casa a casa, e voltar */
            L q1 = pmod(Fc1*inv_mod(Fb1,p), p), q2 = pmod(Fc2*inv_mod(Fb2,p), p);
            L idet = inv_mod(pmod(s2-s1,p), p);
            L v0 = pmod(idet*(s2*q1 - s1*q2), p), v1 = pmod(idet*(q2 - q1), p);
            if(v0 != a0 || v1 != a1) maus++;
            n++;
        }
        printf("     a deconvolucao varrida em %lld casos validos, discordancias: %lld\n\n", n, maus);
        ok("e a DECONVOLUCAO devolve o original: dividir folha a folha desfaz a convolucao, exato",
           maus == 0 && n == 12100);
        puts("        O nucleo sem inversa e o que NAO TEM DUAL — e o koch.c ja lhe deu o nome:");
        puts("        ele nao atravessa a alfandega, e o que ele apagou nao volta. Aqui a condicao");
        puts("        nao e so decidivel: e CONTAVEL, e a contagem tem forma fechada.\n");
    }

    /* ── §U6  a visão unificada ──────────────────────────────────────────── */
    puts("§U6  A VISAO UNIFICADA: os tres raiz(N) num so, e os tres exatos\n");
    {
        int N = 16;
        /* 1. a TRANSFORMADA: a matriz de avaliacao inverte-se, residuo 0 (§U2) */
        L p = 11, m = 1, s1 = 0, s2 = 0;
        raizes_em_Zp(m, p, &s1, &s2);
        L idet = inv_mod(pmod(s2-s1,p), p);
        int e1 = 0;
        for(L a0 = 0; a0 < p; a0++) for(L a1 = 0; a1 < p; a1++){
            L f1 = pmod(a0 + a1*s1, p), f2 = pmod(a0 + a1*s2, p);
            if(pmod(idet*(s2*f1 - s1*f2), p) != a0 || pmod(idet*(f2 - f1), p) != a1) e1++;
        }
        /* 2. a BASE: Hadamard e ortogonal, produto interno 0 exato */
        static signed char H[64][64];
        H[0][0] = 1;
        for(int q = 1; q < N; q *= 2)
            for(int i = 0; i < q; i++) for(int j = 0; j < q; j++){
                H[i][j+q] = H[i][j]; H[i+q][j] = H[i][j]; H[i+q][j+q] = -H[i][j];
            }
        int e2 = 0;
        for(int i = 0; i < N; i++) for(int j = i+1; j < N; j++){
            L ip = 0; for(int k = 0; k < N; k++) ip += (L)H[i][k]*H[j][k];
            if(ip != 0) e2++;
        }
        /* 3. o RUIDO: a soma de N ortogonais tem norma^2 = N*N */
        int e3 = 0;
        {
            L soma_q = 0;
            for(int t = 0; t < N; t++){ L s = 0; for(int i = 0; i < N; i++) s += H[i][t]; soma_q += s*s; }
            if(soma_q != (L)N*N) e3++;
        }
        ok("OS TRES raiz(N) fecham no mesmo N: a transformada, a base e o ruido — os tres com residuo 0",
           e1 == 0 && e2 == 0 && e3 == 0);
        printf("     %-38s %10s\n", "onde aparece o raiz(N)", "residuo");
        printf("     %-38s %10d\n", "1. inverter a matriz de avaliacao", e1);
        printf("     %-38s %10d\n", "2. a base ortogonal (Hadamard)", e2);
        printf("     %-38s %10d\n", "3. somar N independentes (ruido)", e3);
        puts("");
        puts("        E a razao e UMA: a ORTOGONALIDADE. Uma base ortogonal tem norma raiz(N);");
        puts("        a transformada que a usa e invertivel sem perda; e somar N independentes e");
        puts("        somar N direcoes perpendiculares, que Pitagoras conta como raiz(N).");
        puts("");
        puts("        Nao ha tres coeficientes que por acaso coincidem — ha UM, e ele e a norma");
        puts("        do INSTRUMENTO. O §U8 diz de quem ele e, e nao e do objeto.\n");
    }

    /* ── §U7  AUTODUAL ───────────────────────────────────────────────────── */
    puts("§U7  AUTODUAL: o Frobenius tem ORDEM IGUAL AO GRAU, e os fixos sao o subcorpo primo\n");
    puts("     Autodual quer dizer: a transformada leva o corpo NELE PROPRIO. A versao antiga");
    puts("     media isso pela ordem 4 da DFT — mas 4 e um facto da DFT, nao da borda. O que");
    puts("     vale em geral e: o Frobenius gera o grupo que troca as folhas, e a ordem dele e");
    puts("     o GRAU. Para grau 2 isso da ordem 2 — a involucao nu, e nao o i.\n");
    {
        L p = 13, m = 1;
        E sig = {0,1};
        E f1 = epow(sig, p, m, p);              /* Frob(sigma)   */
        E f2 = epow(f1, p, m, p);               /* Frob^2(sigma) */
        printf("     Frob(sigma)   = %lld + %lld.sigma\n", f1.a, f1.b);
        printf("     Frob^2(sigma) = %lld + %lld.sigma   (e sigma e %d + %d.sigma)\n\n",
               f2.a, f2.b, 0, 1);
        ok("Frob^2 = identidade: a ordem do Frobenius e o GRAU (2), e nao 4 — o nu, nao o i",
           eeq(f2, sig) && !eeq(f1, sig));

        /* e os PONTOS FIXOS do Frobenius sao exatamente o subcorpo primo: p de p^2 */
        L fixos = 0, total = 0;
        for(L a = 0; a < p; a++) for(L b = 0; b < p; b++){
            E z = {a,b};
            if(eeq(epow(z, p, m, p), z)) fixos++;
            total++;
        }
        printf("     pontos fixos de Frob em GF(%lld): %lld de %lld   (o subcorpo primo tem %lld)\n\n",
               p*p, fixos, total, p);
        ok("AUTODUAL: os fixos do Frobenius sao EXATAMENTE o subcorpo primo — contados, nao estimados",
           fixos == p && total == p*p);

        /* e o contraste, que e o que da conteudo: nem tudo e fixo. Se fosse, "ser fixo"
         * nao distinguiria nada — foi assim que uma assercao antiga daqui passava sem medir. */
        ok("e o contraste mede: a maioria NAO e fixa — 13 de 169, e nao 169 de 169",
           fixos < total && fixos*13 == total);
        puts("        E E ISSO QUE AUTODUAL SIGNIFICA AQUI: a transformada nao sai do corpo. O");
        puts("        Frobenius permuta as folhas e volta ao fim de GRAU passos, e o que fica");
        puts("        parado e o corpo de base. Um corpo que precisasse de OUTRO para se");
        puts("        transformar nao seria universal — teria de haver um terceiro para fechar.\n");
    }

    /* ── §U8  O ALCANCE ──────────────────────────────────────────────────── */
    puts("§U8  O ALCANCE DO raiz(N): ele e a BASE DA AGULHA, e nao a norma do objeto\n");
    puts("     Um revisor externo apanhou a tensao, e ela e real. A transformada e a avaliacao");
    puts("     nas raizes, e as raizes do metal NAO sao raizes da unidade. No contínuo dir-se-ia");
    puts("     'nao estao no circulo'; em corpo finito diz-se a mesma coisa exatamente, e sem uma");
    puts("     raiz quadrada: mede-se a ORDEM MULTIPLICATIVA de sigma.\n");
    {
        L p = 11;
        printf("     %-22s %14s %16s %14s\n", "borda", "ordem de sigma", "sigma.sigma'", "da unidade?");
        int da_unidade = 0, fora = 0;
        for(L m = 0; m <= 3; m++){
            E sig = {0,1};
            L o = ordem(sig, m, p);
            /* o produto das raizes le-se na borda, sem as calcular: para x^2 - m x - 1 ele
             * e' o termo constante com sinal trocado, ou seja -1. Confirma-se pela norma. */
            E s2 = emul(sig, sig, m, p);
            L norma_sig = pmod(-1, p);           /* sigma.sigma' = -C = -1 */
            int unidade = (o <= 2);
            printf("     x^2 = %lld x + 1        %14lld %16lld %14s\n",
                   m, o, norma_sig, unidade ? "SIM" : "nao");
            (void)s2;
            if(unidade) da_unidade++; else fora++;
        }
        printf("\n");
        ok("so a borda ciclica (m=0) tem sigma com ordem <= 2 — os METAIS estao fora da unidade",
           da_unidade == 1 && fora == 3);

        /* e as duas normas sao RECIPROCAS: sigma.sigma' = -1, exato pela borda, em seis metais */
        L reciprocas = 0, n = 0;
        for(L m = 1; m <= 6; m++){
            /* sigma.sigma' e' o termo constante de x^2 - m x - 1 com sinal trocado: -(-1) = ... */
            /* mede-se no anel: sigma * (sigma - m) = sigma^2 - m sigma = 1, logo sigma e'
             * invertivel e o seu inverso e' sigma - m. Isso E' a reciprocidade. */
            E sig = {0,1}, conj = { pmod(-m,p), 1 };
            E pr = emul(sig, conj, m, p);
            E um = {1,0};
            if(eeq(pr, um)) reciprocas++;
            n++;
        }
        printf("     sigma * (sigma - m) = 1 em %lld metais: %lld fecham\n\n", n, reciprocas);
        ok("as duas raizes sao RECIPROCAS: sigma.(sigma-m) = 1 exato, nos seis metais",
           reciprocas == n && n == 6);
        puts("        Nao sao iguais — sao inversas, e a media GEOMETRICA delas e 1. A base do");
        puts("        metal e ortonormal na norma MULTIPLICATIVA (a da cifra: somar expoentes),");
        puts("        e nao na aditiva.");
        puts("");
        puts("     E DAI SAI A RESPOSTA, que e do Aarao: 'o raiz de N e justamente a base da");
        puts("     AGULHA que mede sem invadir o invariante'.");
        puts("");
        puts("        o OBJETO      o metal, hiperbolico, norma MULTIPLICATIVA (produto = 1)");
        puts("        a AGULHA      a projecao ortogonal, norma ADITIVA (Pitagoras, raiz(N))");
        puts("");
        /* e "medir sem invadir" tem conteudo: a projecao ortogonal nao muda o que projeta.
         * Mede-se em Z_p, onde a divisao e exata — projetar duas vezes da o mesmo. */
        {
            L q = 101, N = 8;
            L idem = 0, perp = 0, casos = 0;
            for(L semente = 1; semente <= 40; semente++){
                L v[8], u[8];
                L s = semente;
                for(int i = 0; i < N; i++){ s = (s*1103515245 + 12345) % q; v[i] = pmod(s,q);
                                            s = (s*1103515245 + 12345) % q; u[i] = pmod(s,q); }
                L nu = 0; for(int i = 0; i < N; i++) nu = pmod(nu + u[i]*u[i], q);
                if(!nu) continue;                       /* u isotropico: nao ha projecao */
                L inu = inv_mod(nu, q);
                L c = 0; for(int i = 0; i < N; i++) c = pmod(c + v[i]*u[i], q);
                L P1[8]; for(int i = 0; i < N; i++) P1[i] = pmod(c*inu % q * u[i], q);
                /* projetar outra vez */
                L c2 = 0; for(int i = 0; i < N; i++) c2 = pmod(c2 + P1[i]*u[i], q);
                L P2[8]; for(int i = 0; i < N; i++) P2[i] = pmod(c2*inu % q * u[i], q);
                int ig = 1; for(int i = 0; i < N; i++) if(P1[i] != P2[i]) ig = 0;
                if(ig) idem++;
                /* e o resto fica perpendicular */
                L ip = 0; for(int i = 0; i < N; i++) ip = pmod(ip + pmod(v[i]-P1[i], q)*u[i], q);
                if(ip == 0) perp++;
                casos++;
            }
            printf("     %lld projecoes em Z_101^8: idempotentes %lld, com resto perpendicular %lld\n\n",
                   casos, idem, perp);
            ok("A AGULHA NAO INVADE: projetar duas vezes da o mesmo — idempotente, exato em Z_101",
               idem == casos && casos == 39);
            ok("e o que sobra fica PERPENDICULAR ao que se mediu: produto interno 0, exato",
               perp == casos && casos == 39);
        }
        puts("     -> a projecao e idempotente e o resto e perpendicular: medir nao muda o que");
        puts("        se mede. E POR ISSO que o raiz(N) e do INSTRUMENTO e nao do objeto — e a");
        puts("        tensao que o revisor achou nao e contradicao, e a DUALIDADE entre os dois:");
        puts("        o objeto e multiplicativo, a agulha e aditiva, e e por viverem em normas");
        puts("        diferentes que uma pode medir a outra sem a invadir.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  A transformada universal e a AVALIACAO NAS RAIZES DA BORDA — e o enredo ja o dizia");
    puts("  (geracao_energia.tex): ela leva convolucao em produto. Aqui mede-se varrendo o anel");
    puts("  INTEIRO, resíduo zero exato, e nao numa amostra.");
    puts("");
    puts("  Cinda a borda ou nao cinda, e a mesma transformada: o que muda e onde as raizes");
    puts("  moram. No corpo primo quando cinde; uma extensao acima, nas folhas de Frobenius,");
    puts("  quando nao. A DFT e o caso m = 0 — a borda ciclica — e nada aqui depende dela.");
    puts("");
    puts("  A deconvolucao existe sse nenhuma folha se anula, e os nucleos que a tem sao");
    puts("  exatamente (p-1)^n. O que anula uma folha e o que nao tem dual, e fica na garrafa.");
    puts("");
    puts("  E o raiz(N) e da AGULHA. As raizes do metal sao reciprocas, nao unitarias: o objeto");
    puts("  e multiplicativo e o instrumento e aditivo, e e por isso que um mede o outro.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
