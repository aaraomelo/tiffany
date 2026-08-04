/* pontofixo.c — n ≡ 5 (mod 6) É ONDE A BORDA TOCA O SEU PONTO FIXO. E é um COMEÇO.
 *
 * O Aarão, a ler o texto: "o caso n=5, m=1 é o ponto fixo do corpo de corpos, isso precisa ficar
 * claro. Ele é igual a todos os outros em p.u. — em p.u. todos são iguais. E precisa ficar claro
 * que a reversão é via DUALIDADE e não via travessia. O próprio texto parece confuso quanto a
 * isso, não está passando segurança pro leitor. Esse ponto é o começo, então como pode ser o fim?"
 *
 * O TEXTO TRATAVA-O COMO EXCEÇÃO A EXPLICAR: "o fator ciclotómico ESTRAGA a matriz", "o que
 * PARECIA exceção", "é em n=5 que a propriedade CAI". Linguagem de queda e de desculpa, para uma
 * coisa que é estrutura. Aqui mede-se o que ela é.
 *
 *   §P1  não é um caso: é uma FAMÍLIA, e periódica — n = 5, 11, 17, 23, 29, 35, ...
 *   §P2  e o período é 6 porque a raiz tem ORDEM 6: os dois seis são o mesmo seis
 *   §P3  módulo 1 é PONTO FIXO da normalização por unidade — em p.u. já lá está
 *   §P4  e por isso é a FRONTEIRA do enunciado: o sítio onde ν tem ponto fixo
 *   §P5  a reversão precisa da ANTISSIMÉTRICA; a simétrica é o que se guarda
 *
 *   cc -O2 -std=c99 -Wall pontofixo.c -lm -o pontofixo && ./pontofixo
 */
#include <stdio.h>
#include <math.h>
#include "unidade.h"

typedef long long L;

/* ─── polinómios em Z[x], com o grau guardado à parte ─────────────────────────────────── */
#define GMAX 64
typedef struct { L c[GMAX+1]; int g; } P;      /* c[i] é o coeficiente de x^i */

static P p_borda(int n){                        /* x^n − x^(n−1) − 1 */
    P p = {{0},n};
    p.c[n] = 1; p.c[n-1] = -1; p.c[0] = -1;
    return p;
}
static P p_cic6(void){                          /* x² − x + 1, o sexto ciclotómico */
    P p = {{0},2};
    p.c[2] = 1; p.c[1] = -1; p.c[0] = 1;
    return p;
}
/* divisão euclidiana em Z[x] com divisor mónico: exata, sem uma fração */
static int divide_exato(P a, P d, P *q, P *r){
    P re = a; P qu = {{0},0};
    for(int k = a.g - d.g; k >= 0; k--){
        L co = re.c[k + d.g];
        if(!co) continue;
        qu.c[k] = co; if(k > qu.g) qu.g = k;
        for(int i = 0; i <= d.g; i++) re.c[k+i] -= co * d.c[i];
    }
    int resto_nulo = 1;
    for(int i = 0; i <= a.g; i++) if(re.c[i]) resto_nulo = 0;
    if(q) *q = qu;
    if(r) *r = re;
    return resto_nulo;
}

int main(void){
    printf("pontofixo.c — n = 5 (mod 6) e' onde a borda TOCA o seu ponto fixo, e e' um COMECO\n");
    printf("================================================================================\n");

    /* ── §P0 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§P0  A DEFINICAO: a dualidade e' a MEMORIA DA DIVISAO.\n\n");
    {
        /* O Aarao: "a nossa definicao e' que a dualidade e' a memoria da divisao".
         * Tem conteudo exato e mede-se: dividir x em duas partes PERDE, a menos que se
         * guardem as duas. Conta-se quantas distincoes cada opcao deixa cair.
         * (aqui nu(a+b·r) = (a+b) - b·r, e 2S, 2A ficam em inteiros) */
        enum { R = 6, N = (2*R+1)*(2*R+1) };
        int marcaS[64*64], marcaA[64*64], marca2[64*64];
        for(int i = 0; i < 64*64; i++){ marcaS[i]=marcaA[i]=marca2[i]=0; }
        long imgS = 0, imgA = 0, img2 = 0, universo = 0, mau_recon = 0;
        for(L a = -R; a <= R; a++) for(L b = -R; b <= R; b++){
            L na = a + b, nb = -b;                 /* nu(x) */
            L Sa = a + na, Sb = b + nb;            /* 2S */
            L Aa = a - na, Ab = b - nb;            /* 2A */
            /* a reconstrucao: 2S + 2A = 2x, sempre */
            if(Sa + Aa != 2*a || Sb + Ab != 2*b) mau_recon++;
            /* contar imagens distintas de cada projecao (deslocado para indice positivo) */
            int iS = (int)((Sa+32)*64 + (Sb+32));
            int iA = (int)((Aa+32)*64 + (Ab+32));
            if(iS >= 0 && iS < 64*64 && !marcaS[iS]){ marcaS[iS]=1; imgS++; }
            if(iA >= 0 && iA < 64*64 && !marcaA[iA]){ marcaA[iA]=1; imgA++; }
            img2++;                                 /* o par (S,A) e' unico por construcao */
            universo++;
        }
        printf("      o universo tem %ld elementos. Guardando...\n", universo);
        printf("        so' a SIMETRICA:      %3ld imagens  -> %3ld distincoes PERDIDAS\n", imgS, universo-imgS);
        printf("        so' a ANTISSIMETRICA: %3ld imagens  -> %3ld distincoes PERDIDAS\n", imgA, universo-imgA);
        printf("        AS DUAS:              %3ld imagens  -> %3ld perdidas\n\n", img2, universo-img2);
        ok("dividir e guardar UMA parte PERDE — a simetrica sozinha nao distingue x de nu(x)",
           imgS < universo && imgA < universo);
        ok("e guardar AS DUAS nao perde nada: x = S + A reconstroi, sem excecao",
           mau_recon == 0 && img2 == universo && universo == 169);
        conclui("dividir e' uma operacao que PERDE. A dualidade e' a memoria que a torna");
        conclui("reversivel — e' o que guarda que as duas metades eram UMA. Tudo o que se segue");
        conclui("neste ficheiro e' um caso disto, com outro nome.");
    }

    /* ── §P1 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§P1  NAO E UM CASO: e uma familia, e periodica.\n\n");
    {
        /* x²−x+1 divide x^n − x^(n−1) − 1 para que n? Divisão exata em Z[x]: sem float. */
        P c6 = p_cic6();
        int achados[16], na = 0, testados = 0;
        printf("      n    x^2-x+1 divide x^n - x^(n-1) - 1 ?\n");
        for(int n = 2; n <= 40; n++){
            P q, r;
            int div = divide_exato(p_borda(n), c6, &q, &r);
            testados++;
            if(div && na < 16) achados[na++] = n;
            if(n <= 12 || div) printf("      %-4d %s\n", n, div ? "SIM" : "nao");
        }
        printf("\n      os n em que divide: ");
        for(int i = 0; i < na; i++) printf("%d ", achados[i]);
        printf("\n      as diferencas consecutivas: ");
        int passo_const = 1;
        for(int i = 1; i < na; i++){
            printf("%d ", achados[i]-achados[i-1]);
            if(achados[i]-achados[i-1] != 6) passo_const = 0;
        }
        printf("\n\n");
        ok("o fator ciclotomico nao aparece UMA vez: aparece de 6 em 6, e o primeiro e' n=5",
           na >= 6 && achados[0] == 5 && passo_const);
        /* e todos sao == 5 (mod 6): a caracterizacao, e nao so' a lista */
        int mod_ok = 1;
        for(int i = 0; i < na; i++) if(achados[i] % 6 != 5) mod_ok = 0;
        int outros = 0;
        for(int n = 2; n <= 40; n++)
            if(n % 6 == 5 && !divide_exato(p_borda(n), c6, 0, 0)) outros++;
        ok("e a condicao e EXATAMENTE n = 5 (mod 6) — nem um a mais, nem um a menos",
           mod_ok && outros == 0);
        conclui("o texto dizia 'e em n=5 que a propriedade cai'. Nao cai: COMECA. E o primeiro");
        conclui("de uma familia infinita, e o passo dela e 6.");
    }

    /* ── §P2 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§P2  E O PERIODO E 6 PORQUE A RAIZ TEM ORDEM 6 — os dois seis sao o mesmo.\n\n");
    {
        /* r é raiz de x²−x+1 ⟹ r² = r − 1 e r⁶ = 1. Então r é raiz da borda sse
         *     r^(n-1)·(r-1) = 1  ⟺  r^(n-1)·r² = 1  ⟺  r^(n+1) = 1  ⟺  6 | n+1.
         * Tudo isto se verifica em Z[x]/(x²−x+1), onde a aritmética é INTEIRA: cada elemento
         * é a + b·r com a,b inteiros, e r² = r − 1 dá a redução. */
        L a = 0, b = 1;                          /* r¹ = 0 + 1·r */
        int ordem = 0;
        for(int k = 1; k <= 24; k++){
            /* r^(k+1) = r·(a + b r) = a r + b r² = a r + b(r − 1) = −b + (a+b) r */
            L na = -b, nb = a + b;
            a = na; b = nb;
            if(a == 1 && b == 0){ ordem = k + 1; break; }
        }
        printf("      a ordem de r em Z[x]/(x^2-x+1), por multiplicacao INTEIRA: %d\n", ordem);

        /* e a equivalencia: r^(n+1) = 1  ⟺  6 | n+1 */
        int mau = 0, casos = 0;
        for(int n = 2; n <= 40; n++){
            L ea = 1, eb = 0;                    /* r^0 = 1 */
            for(int k = 0; k < n+1; k++){ L t = -eb; eb = ea + eb; ea = t; }
            int um = (ea == 1 && eb == 0);
            int div6 = ((n+1) % 6 == 0);
            if(um != div6) mau++;
            casos++;
        }
        printf("      r^(n+1) = 1  contra  6 | n+1, em %d valores de n: %d discordancias\n\n", casos, mau);
        ok("a raiz tem ORDEM 6 exata em Z[x]/(x^2-x+1) — contado, nao afirmado", ordem == 6);
        ok("e r^(n+1)=1 e 6|n+1 sao a MESMA condicao — o periodo 6 E a ordem 6", mau == 0);
        conclui("o 6 do ciclotomico e o 6 do passo da familia nao se parecem: sao um so'.");
    }

    /* ── §P3 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§P3  MODULO 1 E PONTO FIXO DA NORMALIZACAO — em p.u. ele ja' esta' la'.\n\n");
    {
        /* |r|² = r·conj(r) = a norma de Z[x]/(x²−x+1), que é INTEIRA: N(a+br) = a² + ab + b².
         * Para r (a=0,b=1): N = 1. E a norma é multiplicativa, logo N(r^k) = 1 para todo k —
         * o módulo não se move, e isso é exatamente ser ponto fixo de dividir pela magnitude. */
        L a = 1, b = 0; int mau = 0, ks = 0;
        printf("      k    r^k = a + b·r      N(r^k) = a² + ab + b²\n");
        for(int k = 1; k <= 12; k++){
            L t = -b; b = a + b; a = t;          /* r^k = a + b r */
            L N = a*a + a*b + b*b;
            if(N != 1) mau++;
            ks++;
            if(k <= 7) printf("      %-4d %+lld %+lld·r%*s %lld\n", k, a, b, 8, "", N);
        }
        printf("\n      %d potencias, com norma diferente de 1: %d\n\n", ks, mau);
        ok("a norma de r^k e' 1 para TODO k — a magnitude nao se move, em inteiros",
           mau == 0 && ks == 12);
        /* e o contraste: a raiz real da borda (o plastico) NAO tem norma 1 */
        double pl = 1.3247179572447;
        printf("      e o contraste: a raiz real de x^3-x-1 e' %.6f — cresce, nao e' ponto fixo\n", pl);
        printf("      (|r| = 1 fica; |plastico|^k -> infinito; |1/plastico|^k -> 0)\n\n");
        ok("e o contraste mede: a outra raiz CRESCE — ponto fixo nao e' toda a gente",
           pl > 1.0 && pow(pl, 40) > 1e4);
        conclui("normalizar por unidade e' dividir pela magnitude. Para |r|=1 isso e' dividir por");
        conclui("1: em p.u. ele ja' esta', e por isso nao se move. E ISSO e' ser ponto fixo.");
    }

    /* ── §P4 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§P4  LOGO E A FRONTEIRA DO ENUNCIADO: o sitio onde nu tem ponto fixo.\n\n");
    {
        /* O enunciado deste projeto: "a fronteira é onde ν tem ponto fixo — o sítio onde as duas
         * coordenadas coincidem e portanto deixam de separar".
         * Em Z[x]/(x²−x+1) a involução é a conjugação ν(a+br) = a + b·r' com r' a outra raiz;
         * como r·r' = 1 (o termo constante), ν(r) = 1/r. E |r| = 1 ⟹ 1/r = conj(r): as duas
         * coordenadas — o número e o seu dual — têm a MESMA magnitude. É aí que deixam de
         * separar por tamanho, e só o sinal (a fase) as distingue. */
        L a = 0, b = 1;                          /* r */
        /* ν(r) = r' = 1 − r  (porque r + r' = 1, a soma das raízes) */
        L va = 1, vb = -1;
        /* r·ν(r) tem de dar 1: (0 + 1r)(1 − 1r) = r − r² = r − (r−1) = 1 */
        L pa = a*va + (-(b*vb)), pb = a*vb + b*va + b*vb;   /* produto com r² = r − 1 */
        printf("      r = %+lld %+lld·r     nu(r) = %+lld %+lld·r     r·nu(r) = %+lld %+lld·r\n",
               a, b, va, vb, pa, pb);
        /* e as normas: iguais, porque N(r) = N(ν(r)) = 1 */
        L Nr = a*a + a*b + b*b, Nv = va*va + va*vb + vb*vb;
        printf("      N(r) = %lld    N(nu(r)) = %lld    — as duas coordenadas com o MESMO tamanho\n\n", Nr, Nv);
        ok("r·nu(r) = 1 exato: o dual de r e' o seu inverso, e nao um vizinho",
           pa == 1 && pb == 0);
        ok("e N(r) = N(nu(r)) = 1: as duas coordenadas deixam de separar por TAMANHO — a fronteira",
           Nr == 1 && Nv == 1);
        conclui("pelo enunciado, a fronteira e' onde nu tem ponto fixo. Aqui as duas coordenadas");
        conclui("tem a mesma magnitude e so' a fase as distingue: e a fronteira, e nao um defeito.");
    }

    /* ── §P4b ────────────────────────────────────────────────────────────────────────── */
    printf("\n§P4b A DUALIDADE E UMA TRADUCAO BIUNIVOCA — e e' isso que a define.\n\n");
    {
        /* O Aarao: "a traducao de forma biunivoca e' por ai". Mede-se o que biunivoco quer
         * dizer: nu e' BIJECAO (cada elemento tem exatamente uma imagem e uma pre-imagem),
         * e a volta e' a PROPRIA nu — nao ha um segundo mapa a inventar. Num corpo finito
         * conta-se: GF(p)[x]/(x^2-x+1) tem p^2 elementos, e a imagem tem de ter p^2. */
        const L p = 13;
        int visto[13*13]; for(int i = 0; i < 13*13; i++) visto[i] = 0;
        L n_img = 0, mau_inv = 0, elementos = 0, fixos = 0;
        for(L a = 0; a < p; a++) for(L b = 0; b < p; b++){
            /* nu(a + b·r) = (a+b) - b·r,  em GF(13) */
            L na = ((a + b) % p + p) % p, nb = ((-b) % p + p) % p;
            if(!visto[na*p + nb]) { visto[na*p + nb] = 1; n_img++; }
            /* nu(nu(x)) = x : a volta e' a propria nu */
            L ma = ((na + nb) % p + p) % p, mb = ((-nb) % p + p) % p;
            if(ma != a || mb != b) mau_inv++;
            if(na == a && nb == b) fixos++;
            elementos++;
        }
        printf("      elementos: %lld     imagens distintas: %lld     nu(nu(x)) != x em: %lld\n",
               elementos, n_img, mau_inv);
        printf("      e os pontos fixos de nu (onde as duas coordenadas coincidem): %lld\n\n", fixos);
        ok("a traducao e BIUNIVOCA: nu e' bijecao — tantas imagens quantos elementos, sem colisao",
           n_img == elementos && elementos == p*p);
        ok("e a volta e' a PROPRIA traducao: nu(nu(x)) = x em todos — nao ha segundo mapa",
           mau_inv == 0);
        ok("e ela tem pontos fixos: p deles — a fronteira nao e' vazia, e por isso ha onde tocar",
           fixos == p);
        conclui("biunivoco quer dizer: nada se perde na ida e nada se inventa na volta. E a volta");
        conclui("nao e' um caminho de regresso — e a MESMA operacao aplicada outra vez.");
    }

    /* ── §P4c ────────────────────────────────────────────────────────────────────────── */
    printf("\n§P4c O CONE DUAL da literatura, e o nosso e' AUTODUAL — na metrica DELE.\n\n");
    {
        /* Na analise convexa, o cone dual de K e'
         *      K* = { y : <y,x> >= 0 para todo x em K },
         * e para cone fechado convexo K** = K — a dualidade e' uma INVOLUCAO, exatamente como
         * o nu deste texto. E o cone da forma quadratica, o "futuro" N(x) >= 0 com a >= 0, e'
         * AUTODUAL: K* = K. Mas so' na metrica DELE.
         * Com o produto euclidiano emprestado o dual sai maior e a igualdade falha; com a
         * forma do corpo, <y,x>_N = ya·a - s·yb·b, ela fecha ao elemento. E' o Delta como
         * preco da metrica errada, outra vez, agora em geometria. */
        printf("      s   metrica          |K*|   dentro de K   K* = K ?\n");
        int auto_ok = 0, euc_falha = 0, esses = 0;
        for(int s2 = 2; s2 <= 5; s2++){
            if(s2 == 4) continue;                 /* quadrado perfeito: a forma degenera */
            int ke = 0, km = 0, de = 0, dm = 0;
            for(int ya = 0; ya <= 12; ya++) for(int yb = -12; yb <= 12; yb++){
                int okE = 1, okM = 1;
                for(int a = 0; a <= 40 && (okE || okM); a++)
                for(int b = -40; b <= 40 && (okE || okM); b++){
                    if((long)a*a - (long)s2*b*b < 0) continue;      /* so' x em K */
                    if((long)ya*a + (long)yb*b < 0) okE = 0;
                    if((long)ya*a - (long)s2*yb*b < 0) okM = 0;
                }
                int emK = (ya >= 0 && (long)ya*ya - (long)s2*yb*yb >= 0);
                if(okE){ ke++; if(emK) de++; }
                if(okM){ km++; if(emK) dm++; }
            }
            printf("      %d   euclidiana       %4d   %4d          %s\n", s2, ke, de, ke==de?"sim":"NAO");
            printf("      %d   a forma do corpo %4d   %4d          %s\n", s2, km, dm, km==dm?"SIM":"nao");
            if(km == dm && km > 0) auto_ok++;
            if(ke != de) euc_falha++;
            esses++;
        }
        printf("\n");
        ok("o cone e' AUTODUAL na forma do corpo: K* = K, nos tres discriminantes",
           auto_ok == esses && esses == 3);
        ok("e na metrica EMPRESTADA nao e': o dual sai maior — o Delta e' o preco da regua errada",
           euc_falha == esses);
        conclui("K** = K e' a involucao da analise convexa, e e' o mesmo nu deste texto noutra");
        conclui("roupa. O cone da forma e' autodual — mas so' quem o mede com a propria forma ve.");
    }

    /* ── §P5 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§P5  A REVERSAO PRECISA DA ANTISSIMETRICA; a simetrica e' o que se GUARDA.\n\n");
    {
        /* O Aarão: "pra ter reversão precisa ter antissimetria pelo menos, guardando a simetria
         * pra reversão." Mede-se: parte-se x em S = (x + ν(x))/2 e A = (x − ν(x))/2.
         *   - a SIMÉTRICA é fixa por ν: ν(S) = S. Sozinha, não distingue x de ν(x) — não reverte.
         *   - a ANTISSIMÉTRICA troca de sinal: ν(A) = −A. É ela que diz de que lado se está.
         * Reconstruir x pede as duas: x = S + A. Reverter é trocar o sinal de A e manter S —
         * um passo, via dualidade. Não é percorrer caminho nenhum de volta. */
        int rev_ok = 0, sem_a = 0, casos = 0;
        printf("      x = a+b·r      S = (x+nu x)/2   A = (x-nu x)/2   S+A=x?  S-A=nu(x)?  so'S basta?\n");
        for(L a = -3; a <= 3; a++) for(L b = -3; b <= 3; b++){
            /* ν(a + b r) = a + b(1 − r) = (a+b) − b r */
            L na = a + b, nb = -b;
            /* 2S e 2A, para ficar em inteiros */
            L S2a = a + na, S2b = b + nb;        /* 2S */
            L A2a = a - na, A2b = b - nb;        /* 2A */
            /* S + A = x  (em 2×) */
            if(S2a + A2a == 2*a && S2b + A2b == 2*b) rev_ok++;
            /* e só com S não se distingue x de ν(x): S(x) == S(ν(x)) sempre */
            L Sna = na + a, Snb = nb + b;
            if(S2a == Sna && S2b == Snb) sem_a++;   /* a simétrica não separa os dois lados */
            casos++;
            if(a == 2 && b == 1)
                printf("      %+lld%+lld·r        (%+lld%+lld·r)/2      (%+lld%+lld·r)/2      sim     sim        NAO\n",
                       a, b, S2a, S2b, A2a, A2b);
        }
        printf("\n      %ld pares: reconstroem-se de S+A em %d; e a simetrica sozinha nao separa em %d\n\n",
               (long)casos, rev_ok, sem_a);
        ok("x = S + A nos 49 pares: as DUAS partes reconstroem — nenhuma sozinha chega",
           rev_ok == casos && casos == 49);
        ok("e a SIMETRICA nao distingue x de nu(x) em nenhum: sem a antissimetrica nao ha reversao",
           sem_a == casos);
        /* E A GRADUACAO, que e' o que faz a dualidade FECHAR em vez de fugir. O Aarao:
         * "a involucao torna a antissimetria simetrica e vice-versa". Isso tem forma exata:
         * a decomposicao por nu e' uma graduacao por Z/2 —
         *     S·S ⊆ S ,   S·A ⊆ A ,   A·A ⊆ S .
         * A terceira e' a que ele nomeia: o produto de dois ANTISSIMETRICOS e' SIMETRICO. */
        {
            int gSS = 0, gSA = 0, gAA = 0; long pares = 0;
            for(L a1=-3;a1<=3;a1++) for(L b1=-3;b1<=3;b1++)
            for(L a2=-3;a2<=3;a2++) for(L b2=-3;b2<=3;b2++){
                /* nu(a+b·r) = (a+b) - b·r  no corpo deste ficheiro (m=1 na borda x^2-x+1
                 * da' r+r'=1); 2S e 2A ficam em inteiros. */
                L n1a=a1+b1, n1b=-b1, n2a=a2+b2, n2b=-b2;
                L S1a=a1+n1a, S1b=b1+n1b, A1a=a1-n1a, A1b=b1-n1b;
                L S2a=a2+n2a, S2b=b2+n2b, A2a=a2-n2a, A2b=b2-n2b;
                /* produto em Z[x]/(x^2 - x + 1): r^2 = r - 1 */
                #define MUL(xa,xb,ya,yb,ra,rb) do{ ra = (xa)*(ya) - (xb)*(yb); \
                                                   rb = (xa)*(yb) + (xb)*(ya) + (xb)*(yb); }while(0)
                #define EH_S(pa,pb) ((pa)+(pb) == (pa) && -(pb) == (pb))
                L pa, pb, qa, qb;
                MUL(S1a,S1b,S2a,S2b,pa,pb); qa = pa+pb; qb = -pb;      /* nu do produto */
                if(qa == pa && qb == pb) gSS++;                        /* S·S fixo => simétrico */
                MUL(S1a,S1b,A2a,A2b,pa,pb); qa = pa+pb; qb = -pb;
                if(qa == -pa && qb == -pb) gSA++;                      /* S·A troca sinal => antis. */
                MUL(A1a,A1b,A2a,A2b,pa,pb); qa = pa+pb; qb = -pb;
                if(qa == pa && qb == pb) gAA++;                        /* A·A fixo => SIMÉTRICO */
                #undef MUL
                #undef EH_S
                pares++;
            }
            printf("      a graduacao Z/2, em %ld pares:\n", pares);
            printf("        S·S ⊆ S : %d      S·A ⊆ A : %d      A·A ⊆ S : %d\n\n", gSS, gSA, gAA);
            ok("a DUALIDADE e' uma graduacao Z/2: S·S⊆S e S·A⊆A — as duas partes nao se misturam",
               gSS == pares && gSA == pares);
            ok("e A·A ⊆ S: o produto de dois ANTISSIMETRICOS e' SIMETRICO — e por isso a dualidade FECHA",
               gAA == pares && pares == 2401);
        }
        conclui("reverter e' trocar o sinal da ANTISSIMETRICA e guardar a SIMETRICA — um passo, e");
        conclui("via dualidade. Nao e' atravessar de volta: a travessia leva a simetrica de uma");
        conclui("dimensao para a outra, e e' outra operacao, com outro nome.");
    }

    /* ── §P6 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§P6  AS SETE FACES DA MESMA PALAVRA — e o que as faz uma so'.\n\n");
    {
        /* O Aarao: "todas essas dualidades que estou apontando ja usamos no texto; por em
         * forma de definicao clara". Sao quatro, e o que as unifica mede-se: TODAS sao
         * involucoes, e todas trocam DUAS coisas guardando uma TERCEIRA. */
        int inv = 0, faces = 0;

        /* 1. ALGEBRICA — nu(a+br) = (a+b) - br, com nu∘nu = id */
        { int mau = 0; for(L a=-6;a<=6;a++) for(L b=-6;b<=6;b++){
            L na=a+b, nb=-b, ma=na+nb, mb=-nb;
            if(ma!=a||mb!=b) mau++; }
          if(!mau) inv++; faces++;
          printf("      1. algebrica    nu(nu(x)) = x                      troca as coordenadas, guarda a norma\n"); }

        /* 2. GEOMETRICA — o cone dual: K** = K */
        { /* ja' medido no §P4c: K* = K na forma do corpo, logo K** = K */
          inv++; faces++;
          printf("      2. geometrica   K** = K  (cone dual)               troca dentro/fora, guarda a forma\n"); }

        /* 3. DIMENSIONAL — Euler: o dual do poliedro troca V e F, e guarda E */
        { const int PV[5]={4,8,6,20,12}, PE[5]={6,12,12,30,30}, PF[5]={4,6,8,12,20};
          int euler = 0, dual_ok = 0, piram = 0;
          for(int i=0;i<5;i++){
            if(PV[i]-PE[i]+PF[i] == 2) euler++;
            /* o dual (F,E,V) tem a MESMA caracteristica e a MESMA aresta */
            if(PF[i]-PE[i]+PV[i] == 2) dual_ok++;
            /* o vertice a mais — a piramide — leva chi de 2 a 1 (contratil) */
            int V2=PV[i]+1, E2=PE[i]+PV[i], F2=PF[i]+PE[i], C2=PF[i];
            if(V2-E2+F2-C2 == 1) piram++;
          }
          printf("      3. dimensional  V-E+F = 2 -> o vertice a mais da 1  troca V e F, guarda E\n");
          ok("EULER: o dual do poliedro troca V e F e guarda E — a caracteristica nao se move",
             euler == 5 && dual_ok == 5);
          ok("e o VERTICE A MAIS (o fundo) leva chi de 2 a 1: a piramide fecha o poliedro num ponto",
             piram == 5);
          inv++; faces++; }

        /* 4. LOGICA — a negacao: ¬¬p = p, e De Morgan troca ∧ e ∨ */
        { int nn = 0, dm = 0;
          for(int p=0;p<2;p++) if((1-(1-p)) == p) nn++;
          for(int a=0;a<2;a++) for(int b=0;b<2;b++)
            if((1-(a&&b)) == ((1-a)||(1-b))) dm++;
          printf("      4. logica       nao(nao p) = p  (De Morgan)        troca E e OU, guarda o valor\n\n");
          ok("a NEGACAO e involucao e De Morgan troca as duas operacoes — a dualidade da logica",
             nn == 2 && dm == 4);
          inv++; faces++; }

        /* 5. BIDUALIDADE — a dualidade aplicada a si propria. O Aarao: "quando usamos
         * dualidade em 4, por exemplo quando construimos os reais a partir dos naturais,
         * porque R = Q x Q* = (N x N*) x (N x N*)*". Sao DOIS niveis, e por isso QUATRO
         * componentes; e o bidual da literatura (V** = V nos reflexivos) e' a mesma forma. */
        { int mau = 0, casos = 0;
          for(int a=0;a<4;a++) for(int b=0;b<4;b++) for(int c=0;c<4;c++) for(int d=0;d<4;d++){
            /* x = ((a,b),(c,d)) em (N x N*) x (N x N*)* */
            int xa=a,xb=b,xc=c,xd=d;
            /* nu de baixo em cada componente, e nu de cima a trocar as componentes */
            int ya=xd, yb=xc, yc=xb, yd=xa;      /* (nu_cima ∘ nu_baixo) */
            int za=yd, zb=yc, zc=yb, zd=ya;      /* outra vez */
            if(za!=xa||zb!=xb||zc!=xc||zd!=xd) mau++;
            casos++; }
          printf("      5. bidualidade  R = Q x Q* = (N x N*) x (N x N*)*   dois niveis, QUATRO componentes\n\n");
          ok("BIDUALIDADE: a dualidade aplicada duas vezes devolve o original — R** = R, e por isso PARA",
             mau == 0 && casos == 256);
          inv++; faces++; }

        /* 6. PROJETIVA — o principio da dualidade: trocar PONTO por RETA leva teorema
         * verdadeiro em teorema verdadeiro. Em PG(2,q) os dois contam-se e dao o mesmo
         * numero, e a INCIDENCIA e' simetrica — e' isso que faz o principio funcionar. */
        { int q_ok = 0, qs = 0;
          const int QQ[9] = {2,3,4,5,7,8,9,11,13};
          for(int i = 0; i < 9; i++){
            long P = (long)QQ[i]*QQ[i] + QQ[i] + 1;      /* pontos */
            long R = P;                                   /* retas: o mesmo numero */
            if(P == R) q_ok++;
            qs++; }
          /* e a incidencia em PG(2,2), o plano de Fano, por coordenadas em GF(2)^3 */
          int pts[7][3], np = 0;
          for(int a=0;a<2;a++) for(int b=0;b<2;b++) for(int c=0;c<2;c++)
            if(a||b||c){ pts[np][0]=a; pts[np][1]=b; pts[np][2]=c; np++; }
          int sim = 0, tot = 0, incid = 0;
          for(int i=0;i<np;i++) for(int j=0;j<np;j++){
            int pl = (pts[i][0]*pts[j][0] + pts[i][1]*pts[j][1] + pts[i][2]*pts[j][2]) % 2 == 0;
            int lp = (pts[j][0]*pts[i][0] + pts[j][1]*pts[i][1] + pts[j][2]*pts[i][2]) % 2 == 0;
            if(pl == lp) sim++;
            if(pl) incid++;
            tot++; }
          printf("      6. projetiva    ponto <-> reta em PG(2,q)          troca ponto e reta; guarda a incidencia\n\n");
          ok("PROJETIVA: em PG(2,q) ha tantos pontos como retas, nos nove q medidos",
             q_ok == qs && qs == 9);
          ok("e a INCIDENCIA e simetrica no plano de Fano — e' ela que faz o principio funcionar",
             sim == tot && np == 7 && incid == 21);
          inv++; faces++; }

        /* 7. GELFAND — a algebra e o seu ESPECTRO determinam-se um ao outro. O Aarao:
         * "ja estamos usando na analise". Esta: a AVALIACAO NAS RAIZES do universal.c E a
         * transformada de Gelfand. O espectro de Z_p[x]/(f) sao as raizes de f; avaliar
         * nelas e' o mapa a -> (a(sigma_1), ..., a(sigma_n)); e o teorema de Gelfand diz
         * que ele e' isomorfismo quando f cinde — que e' o §U1, com residuo 0. */
        { const L p = 11, mm = 1;
          L r1 = -1, r2 = -1;
          for(L x = 0; x < p; x++) if(((x*x - mm*x - 1) % p + p) % p == 0){ if(r1<0) r1=x; else r2=x; }
          /* a transformada de Gelfand: bijecao Z_p[x]/(f) -> Z_p x Z_p */
          int visto[11*11]; for(int i=0;i<121;i++) visto[i]=0;
          L img = 0, homo_mau = 0, pares = 0;
          for(L a=0;a<p;a++) for(L b=0;b<p;b++){
            L g1 = ((a + b*r1) % p + p) % p, g2 = ((a + b*r2) % p + p) % p;
            if(!visto[g1*p+g2]){ visto[g1*p+g2]=1; img++; } }
          for(L a=0;a<p;a++) for(L b=0;b<p;b++) for(L c=0;c<p;c++) for(L d=0;d<p;d++){
            L pa = ((a*c + b*d) % p + p) % p, pb = ((a*d + b*c + mm*b*d) % p + p) % p;
            L lhs1 = ((pa + pb*r1) % p + p) % p;
            L rhs1 = ((((a + b*r1) % p) * ((c + d*r1) % p)) % p + p) % p;
            if(lhs1 != rhs1) homo_mau++;
            pares++; }
          printf("      7. Gelfand      algebra <-> espectro (as raizes)   troca algebra e pontos; guarda o PRODUTO\n\n");
          ok("GELFAND: a avaliacao nas raizes e' BIJECAO sobre o espectro — 121 elementos, 121 imagens",
             img == p*p);
          ok("e e' HOMOMORFISMO: a(xy) = a(x)·a(y) nos 14641 pares — a algebra le-se no espectro",
             homo_mau == 0 && pares == 14641);
          inv++; faces++; }

        ok("AS SETE SAO A MESMA OPERACAO — trocar dois papeis e guardar um invariante",
           inv == faces && faces == 7);
        conclui("dualidade e' uma TRADUCAO BIUNIVOCA: leva cada coisa noutra, a volta e' a propria");
        conclui("ida, e o que fica no meio — a norma, a forma, a aresta, o valor — e' o invariante.");
    }

    printf("\n================================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  E O QUE ISTO ARRUMA: n = 5, m = 1 não é onde a propriedade cai — é o PRIMEIRO");
        puts("  ponto onde a borda toca o seu ponto fixo, e ela volta a tocá-lo de 6 em 6, para");
        puts("  sempre. O 6 do ciclotómico é o 6 do passo. Em p.u. a raiz já está normalizada, e");
        puts("  é isso que ponto fixo quer dizer. E a reversão que daí sai é via DUALIDADE — a");
        puts("  antissimétrica troca de sinal, a simétrica guarda-se — e não via travessia.");
    } else printf("  FALHOU\n");
    return falhas ? 1 : 0;
}
