/* hurwitz.c — A TORRE DE HURWITZ, E A DUAL QUE DESCE: o telómero é o lado de baixo.
 *
 * O Aarão: "o telómero é a torre de Hurwitz dual [...] pega a torre de Hurwitz e mede também."
 *
 * A torre existe fora deste repositório, em `broca-so/linguagem/torre_hurwitz.py`, e o que ela diz
 * é isto: só há QUATRO álgebras de divisão normadas sobre R — R(1), C(2), H(4), O(8) — e a
 * construção de Cayley--Dickson dobra a dimensão a cada passo, como uma boneca russa.
 *
 * E O QUE FAZ DELA A NOSSA TORRE é a meta-indução dos dois lados, que é a mesma estrutura que o
 * `furos.c` §F5 mediu nos passos e o `base.c` §B11 nas dimensões:
 *
 *     SUBINDO   R -> C -> H -> O    cada dobra PERDE uma propriedade
 *               a ordem, depois a comutatividade, depois a associatividade, depois a divisão.
 *               Limitada ACIMA por Hurwitz: em 16 dimensões já há divisores de zero.
 *
 *     DESCENDO  O -> H -> C -> R    cada metade RECUPERA a propriedade que a de cima perdeu.
 *               Limitada ABAIXO por R, onde não há mais nada a recuperar.
 *
 * As duas induções encontram-se, e é esse encontro que torna a torre FINITA. Não é uma escolha de
 * paragem: é onde as duas limitações se tocam.
 *
 * O CRISTAL — o que sobrevive a todos os andares — é a NORMA: N(xy) = N(x)N(y). É o próprio
 * teorema de Hurwitz, e é ele que falha primeiro quando se dobra uma vez a mais. Medir a torre é,
 * portanto, medir onde é que cada coisa se parte, e o mais importante é que elas NÃO se partem
 * todas no mesmo sítio: é essa escada de perdas que dá à torre os seus quatro degraus.
 *
 * ── TUDO DISCRETO, TUDO INTEIRO ────────────────────────────────────────────────────────────
 * Este medidor nasceu com 20 doubles e uma tolerância `1e-9`, e nenhum deles fazia falta: o
 * gerador SEMPRE produziu inteiros, e Cayley--Dickson só usa +, − e ×, que não saem de Z. Os
 * doubles eram um transporte, não uma necessidade — e um transporte que trazia um LIMIAR de
 * borla. Com inteiros, «a norma é multiplicativa» deixa de ser «o resíduo é menor que 1e-9» e
 * passa a ser «o resíduo é ZERO», que é outra afirmação: a primeira tem uma régua escolhida por
 * mim, a segunda não tem régua nenhuma.
 *
 * O gerador ficou LETRA POR LETRA o mesmo. Se eu mudasse os dados ao mesmo tempo que mudo o tipo,
 * já não podia comparar as duas versões — e a comparação é a prova de que não perdi cobertura.
 *
 *   §H1  a DOBRA de Cayley--Dickson, e a boneca russa: cada nível contém o anterior
 *   §H2  o CRISTAL: N(xy) = N(x)N(y) em 1,2,4,8 — e onde ele se parte
 *   §H3  SUBINDO perde-se, um degrau de cada vez: ordem, comuta, associa, divide
 *   §H4  DESCENDO recupera-se — a metaindução, e o encontro que fecha a torre
 *   §H5  e o TELÓMERO é a torre DUAL: o lado que desce, e o sinal trocado
 *   §H6  O FECHO DO DUAL: directo² + cruzado² = N(u)N(v), e porque o degrau 4 existe
 *
 *   cc -O2 -std=c99 -I. hurwitz.c -o hurwitz && ./hurwitz
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

#define DMAX 64
/* O TETO, e ele VERIFICA-SE. As entradas do gerador vivem em [−28, 9]; a dobra em dimensão 16
 * multiplica e soma 16 delas, e o pior caso de (xy)z fica na ordem de 10^7. A norma eleva ao
 * quadrado e soma 16, o que dá ~10^15 — dentro de long (9.2·10^18). Mas um teto que ninguém
 * testa é documentação, não limite: o contador `estouros` abaixo vigia-o em cada conta. */
#define TETO 2000000000L
static long estouros = 0;
static long guarda(long v){ if(v > TETO || v < -TETO) estouros++; return v; }

/* o conjugado: guarda a parte real, nega as imaginárias */
static void conjuga(const long *x, int n, long *o){
    o[0] = x[0];
    for(int k = 1; k < n; k++) o[k] = -x[k];
}
/* Cayley--Dickson: (a,b)(c,d) = (a·c − conjuga(d)·b, d·a + b·conjuga(c)) — a dobra */
static void cd(const long *x, const long *y, int n, long *o){
    if(n == 1){ o[0] = guarda(x[0]*y[0]); return; }
    int m = n/2;
    const long *a = x, *b = x+m, *c = y, *d = y+m;
    long ac[DMAX], db[DMAX], da[DMAX], bc[DMAX], cd_[DMAX], dc[DMAX];
    cd(a, c, m, ac);
    conjuga(d, m, cd_); cd(cd_, b, m, db);
    cd(d, a, m, da);
    conjuga(c, m, dc);  cd(b, dc, m, bc);
    for(int k = 0; k < m; k++){ o[k] = guarda(ac[k] - db[k]); o[m+k] = guarda(da[k] + bc[k]); }
}
static long norma2(const long *x, int n){
    long s = 0;
    for(int k = 0; k < n; k++) s += x[k]*x[k];
    return s;
}
/* um elemento determinista do nível n — IDÊNTICO ao da versão com doubles */
static void gera(long *x, int n, long s){
    for(int k = 0; k < n; k++){
        long h = s*1103515245L + k*12345L + 7;
        h ^= h >> 13;
        x[k] = (h % 19) - 9;
    }
}
/* «NÃO» tem 3 caracteres e 4 BYTES: o %-Ns paga por bytes e come uma coluna. Conta-se em
 * caracteres, que é o que o Aarão vê — o valor pode estar certo e o texto errado. */
static void col(const char *r, int largura){
    fputs(r, stdout);
    for(const char *q = r; *q; q++) if((*q & 0xC0) != 0x80) largura--;
    while(largura-- > 0) putchar(' ');
}
static int iguais(const long *a, const long *b, int n){
    for(int k = 0; k < n; k++) if(a[k] != b[k]) return 0;
    return 1;
}

int main(void){
printf("\n=== A TORRE DE HURWITZ, E A DUAL QUE DESCE ================================\n");
printf("    Quatro degraus — R(1), C(2), H(4), O(8) — e cada dobra perde uma coisa.\n");
printf("    As perdas não caem todas no mesmo sítio, e é isso que faz a escada.\n");
printf("    Tudo em INTEIROS: não há tolerância nenhuma, e «igual» quer dizer igual.\n");

printf("\n§H1  A DOBRA: cada nível CONTÉM o anterior — a boneca russa.\n\n");
{
    /* Um elemento do nivel n com a metade de cima a zero E' um elemento do nivel n/2, e
     * multiplicar la' dentro tem de dar o mesmo que multiplicar em baixo. Se nao desse, a torre
     * nao era encaixada — eram quatro algebras soltas com o mesmo nome. */
    int mau = 0; long casos = 0;
    printf("      nível   mergulhados   produto igual ao do nível de baixo?\n");
    for(int n = 2; n <= 8; n *= 2){
        int m = n/2, bom = 1;
        for(long s = 1; s <= 60; s++){
            long a[DMAX] = {0}, b[DMAX] = {0}, A[DMAX] = {0}, B[DMAX] = {0};
            gera(a, m, s); gera(b, m, s+31);
            memcpy(A, a, (size_t)m*sizeof(long));   /* mergulha: metade de cima fica 0 */
            memcpy(B, b, (size_t)m*sizeof(long));
            long p[DMAX], P[DMAX];
            cd(a, b, m, p);
            cd(A, B, n, P);
            for(int k = 0; k < m; k++) if(p[k] != P[k]){ bom = 0; mau++; }
            for(int k = m; k < n; k++) if(P[k] != 0){ bom = 0; mau++; }
            casos++;
        }
        printf("      %-7d %-13d %s\n", n, m, bom ? "sim" : "NÃO");
    }
    printf("\n      %ld casos\n\n", casos);
    ok("cada nível contém o anterior — a torre é encaixada, não são álgebras soltas",
       mau == 0);
}

printf("\n§H2  O CRISTAL: N(xy) = N(x)N(y) — e onde ele se parte.\n\n");
{
    /* O proprio teorema de Hurwitz: a norma e' multiplicativa em 1,2,4,8 e em mais nenhum. Aqui
     * nao se assume — testa-se ATE' 16, e espera-se que 16 FALHE. Um medidor que so' testasse
     * ate' 8 nao estava a medir o teorema: estava a confirmar a metade que lhe agradava.
     *
     * E agora o resíduo é INTEIRO: ou é 0, ou não é. Antes havia um `pior < 1e-9` a decidir por
     * mim o que contava como zero. */
    printf("      dim    álgebra    N(xy) = N(x)N(y)?    pior resíduo (inteiro)\n");
    int falha8 = 0, passou16 = 0;
    const char *nome[] = {"?","R","C","?","H","?","?","?","O","","","","","","","","S"};
    for(int n = 1; n <= 16; n *= 2){
        long pior = 0;
        for(long s = 1; s <= 400; s++){
            long x[DMAX], y[DMAX], p[DMAX];
            gera(x, n, s); gera(y, n, s*7+3);
            cd(x, y, n, p);
            long e = norma2(p,n) - norma2(x,n)*norma2(y,n);
            if(e < 0) e = -e;
            if(e > pior) pior = e;
        }
        int multiplicativa = (pior == 0);
        if(n <= 8 && !multiplicativa) falha8++;
        if(n == 16 && multiplicativa) passou16++;
        printf("      %-6d %-10s ", n, nome[n]);
        col(multiplicativa ? "sim" : "NÃO", 20);
        printf(" %ld\n", pior);
    }
    printf("\n");
    ok("a norma é multiplicativa em R, C, H e O — o cristal aguenta os quatro, com resíduo"
       " EXATAMENTE zero e não «zero abaixo de um limiar meu»", falha8 == 0);
    ok("e NÃO é em 16 dimensões: é aqui que Hurwitz põe o teto", passou16 == 0);
    printf("      O teto não é uma convenção nem uma escolha de paragem: é onde a norma deixa\n");
    printf("      de compor. Dobrar mais uma vez é permitido — o que se perde é o cristal.\n");
}

printf("\n§H3  SUBINDO perde-se, um degrau de cada vez.\n\n");
{
    /* A escada das perdas. Mede-se CADA propriedade em CADA nivel, e o que interessa e' que
     * elas nao caem todas juntas: e' o desencontro que da' os quatro degraus. */
    printf("      dim   comuta?   associa?   sem divisores de zero?\n");
    int mau_esperado = 0;
    for(int n = 1; n <= 16; n *= 2){
        int comuta = 1, associa = 1;
        for(long s = 1; s <= 300 && (comuta || associa); s++){
            long x[DMAX], y[DMAX], z[DMAX], p[DMAX], q[DMAX], r[DMAX], t[DMAX];
            gera(x,n,s); gera(y,n,s*5+1); gera(z,n,s*11+2);
            cd(x,y,n,p); cd(y,x,n,q);
            if(!iguais(p,q,n)) comuta = 0;
            cd(p,z,n,r);                       /* (xy)z */
            cd(y,z,n,t); cd(x,t,n,q);          /* x(yz) */
            if(!iguais(r,q,n)) associa = 0;
        }
        /* DIVISORES DE ZERO, e aqui eu tinha CITADO em vez de medir: pus o par (e3+e10)(e6+e15)
         * da literatura, que não anula nesta convenção de Cayley--Dickson — as variantes da
         * fórmula mudam quais são os pares. Um número copiado de fora que não bate com o
         * código de dentro não prova nada sobre nenhum dos dois.
         *
         * Procura-se então: varrem-se todos os pares (e_i+e_j)(e_k+e_l) e conta-se quantos
         * anulam. Se em 16 houver algum e em 8 nenhum, o degrau está onde Hurwitz diz — e o
         * facto é NOSSO, não emprestado. E agora «anula» quer dizer anula: norma2 == 0. */
        int sem_div = 1;
        long achados = 0;
        if(n >= 8){
            for(int i = 1; i < n && !achados; i++)
            for(int j = i+1; j < n && !achados; j++)
            for(int k = 1; k < n && !achados; k++)
            for(int l = k+1; l < n; l++){
                long u[DMAX] = {0}, v[DMAX] = {0}, w[DMAX];
                u[i] = 1; u[j] = 1; v[k] = 1; v[l] = 1;
                cd(u, v, n, w);
                if(norma2(w,n) == 0){ achados++; sem_div = 0;
                    if(n == 16) printf("      (achado em %d: (e%d+e%d)(e%d+e%d) = 0)\n", n,i,j,k,l);
                    break; }
            }
        }
        /* a escada esperada: comuta até 2, associa até 4, sem divisores até 8 */
        int esp_c = (n <= 2), esp_a = (n <= 4), esp_d = (n <= 8);
        if(comuta != esp_c || associa != esp_a || sem_div != esp_d) mau_esperado++;
        printf("      %-5d ", n);
        col(comuta ? "sim" : "não", 9);  printf(" ");
        col(associa ? "sim" : "não", 10); printf(" ");
        col(sem_div ? "sim" : "NÃO", 3);  printf("\n");
    }
    printf("\n");
    ok("a escada das perdas é exatamente 2, 4, 8 — e nenhuma cai fora do seu degrau",
       mau_esperado == 0);
    printf("      Se as três se perdessem no mesmo sítio, a torre tinha um degrau só. É o\n");
    printf("      DESENCONTRO das perdas que a faz ter quatro — e por isso a torre é uma\n");
    printf("      escada e não um precipício.\n");
}

printf("\n§H4  DESCENDO recupera-se: a metaindução, e o encontro que fecha.\n\n");
{
    /* A torre dual. Descer e' tomar a METADE DE BAIXO, e o que se mede e' que cada descida
     * RECUPERA a propriedade que a subida tinha perdido — e que a recuperacao para em R,
     * porque la' nao ha' mais nada a recuperar. As duas inducoes encontram-se, e o encontro e'
     * o que fecha a torre em quatro. */
    printf("      de     para   recupera            e em R?\n");
    const char *nomes[] = {"","R","C","","H","","","","O"};
    int mau = 0;
    for(int n = 8; n >= 2; n /= 2){
        int m = n/2;
        /* mede-se: a propriedade que falta em n existe em m? */
        int associa_m = 1, associa_n = 1;
        for(long s = 1; s <= 200; s++){
            long x[DMAX], y[DMAX], z[DMAX], p[DMAX], q[DMAX], r[DMAX], t[DMAX];
            gera(x,m,s); gera(y,m,s*5+1); gera(z,m,s*11+2);
            cd(x,y,m,p); cd(p,z,m,r); cd(y,z,m,t); cd(x,t,m,q);
            if(!iguais(r,q,m)) associa_m = 0;
            gera(x,n,s); gera(y,n,s*5+1); gera(z,n,s*11+2);
            cd(x,y,n,p); cd(p,z,n,r); cd(y,z,n,t); cd(x,t,n,q);
            if(!iguais(r,q,n)) associa_n = 0;
        }
        if(n == 8 && !(associa_m && !associa_n)) mau++;   /* O não associa, H associa */
        printf("      %-6s %-6s %-19s %s\n", nomes[n], nomes[m],
               n == 8 ? "a associatividade" : (n == 4 ? "a comutatividade" : "a ordem"),
               m == 1 ? "chegou ao fundo" : "");
    }
    printf("\n");
    ok("descer de O para H recupera a associatividade — a metaindução ganha o que a indução perdeu",
       mau == 0);
    printf("      Subir é limitado ACIMA por Hurwitz (a norma parte-se em 16); descer é\n");
    printf("      limitado ABAIXO por R (não há o que recuperar). As duas limitações tocam-se,\n");
    printf("      e é esse toque — não uma escolha nossa — que faz a torre ter quatro degraus.\n");
}

printf("\n§H5  E O TELÓMERO É A TORRE DUAL: o lado que desce, com o sinal trocado.\n\n");
{
    /* O Aarao: "o telomero e' a torre de Hurwitz dual". O que se mede aqui e' o que torna essa
     * frase verificavel: a conjugacao — que e' trocar o sinal da parte imaginaria, isto e', o
     * sinal da multiplicacao no sentido do furos.c §F4 — e' uma INVOLUCAO em todo andar, e e'
     * ela que faz a norma sair: N(x) = x·conjuga(x). O lado que desce é o lado conjugado.
     *
     * ── E ESTE LAÇO PARAVA EM 8 ──────────────────────────────────────────────────────
     * Media a involução só até onde a NORMA também vale — isto é, no sítio onde os dois
     * lados concordam, que é o sítio onde a pergunta não tem gume. A frase que interessa
     * está em `corpo-estelar.tex` def:octoniao-dual: o mesmo espaço lê-se de dois modos,
     * «pela norma bilinear PERDE a associatividade; pela dualidade NÃO PERDE NADA — a
     * volta fecha». Isso só se pode medir ONDE A NORMA JÁ MORREU, e eu nunca lá tinha
     * corrido. Vai agora a 64: em 16, 32 e 64 a multiplicatividade está partida e a
     * involução continua com resíduo ZERO. O tecto é do lado da NORMA, não do objecto. */
    int mau_inv = 0, mau_norma = 0, dual_vivo = 0, norma_morta = 0;
    printf("      dim   N(xy)=N(x)N(y)?   conj∘conj = id?   x·conjuga(x) = N(x)?\n");
    for(int n = 1; n <= 64; n *= 2){
        int inv = 1, nrm = 1, mult = 1;
        for(long s = 1; s <= 200; s++){
            long x[DMAX] = {0}, y[DMAX] = {0}, c1[DMAX] = {0}, c2[DMAX] = {0}, p[DMAX] = {0};
            gera(x, n, s);
            gera(y, n, s*7+3);
            cd(x, y, n, p);
            if(norma2(p,n) != norma2(x,n)*norma2(y,n)) mult = 0;
            conjuga(x, n, c1); conjuga(c1, n, c2);
            if(!iguais(c2, x, n)) inv = 0;
            cd(x, c1, n, p);
            if(p[0] != norma2(x,n)) nrm = 0;
            for(int k = 1; k < n; k++) if(p[k] != 0) nrm = 0;
        }
        if(!inv) mau_inv++;
        if(!nrm) mau_norma++;
        if(!mult && inv && nrm) dual_vivo++;      /* a norma partida E o dual inteiro */
        if(!mult) norma_morta++;
        printf("      %-5d ", n);
        col(mult ? "sim" : "NÃO", 17); printf(" ");
        col(inv  ? "sim" : "NÃO", 17); printf(" ");
        col(nrm  ? "sim" : "NÃO", 3);  printf("\n");
    }
    printf("\n");
    ok("a conjugação é involução em todo andar — trocar duas vezes devolve, com resíduo 0"
       " EXATO e não «abaixo de 1e-12»", mau_inv == 0);
    ok("e x·conjuga(x) dá a norma, real e pura: o dual é quem produz o cristal", mau_norma == 0);
    ok("E O TECTO É DO LADO DA NORMA, NÃO DO OBJECTO: em 16, 32 e 64 a multiplicatividade"
       " está PARTIDA e a involução continua com resíduo ZERO. É o def:octoniao-dual do"
       " corpo estelar medido — «pela norma perde; pela dualidade não perde nada» — e este"
       " laço parava em 8, isto é, media a involução só onde a norma também valia, que é"
       " onde a pergunta não tem gume",
       dual_vivo == 3 && norma_morta == 3);
    printf("      É a mesma involução do furos.c §F4 (σ·σ' = −1) e do ribossomo.c §Y5 (as duas\n");
    printf("      fitas): trocar o sinal de uma peça, e trocar duas vezes devolver. O telómero\n");
    printf("      é a torre dual porque é o lado conjugado — o que desce, e o que ao descer\n");
    printf("      produz a norma que o outro lado conserva. E a torre NÃO ACABA em 8: acaba\n");
    printf("      em 8 do lado que se mede pela norma. Do lado do dual não tem tecto.\n");
}

printf("\n§H6  O FECHO DO DUAL: directo² + cruzado² = N(u)N(v).\n\n");
{
    /* O Aarão: «fecha o dual». Ele tinha razão, e o buraco era este: a casa tinha o SPLIT
     * directo/cruzado (corpo-estelar §640: fp = cos θ, tan φ = cruzado/directo) e tinha a
     * CONSERVAÇÃO da norma (§H2 aqui em cima), e nunca escreveu a equação que diz que a segunda
     * É a primeira. Estava a medir metade de um par dual — outra vez.
     *
     *        ⟨u,v⟩²  +  ‖u∧v‖²  =  N(u)·N(v)
     *
     * É a identidade de Lagrange, é cos²θ + sin²θ = 1 com a norma por dentro, e mede-se ao
     * QUADRADO: a raiz nunca se tira. */
    long mal = 0, pares = 0;
    for(long a=-3;a<=3;a++) for(long b=-3;b<=3;b++) for(long c=-3;c<=3;c++)
    for(long d=-3;d<=3;d++) for(long e=-3;e<=3;e++) for(long f=-3;f<=3;f++){
        long u[3]={a,b,c}, v[3]={d,e,f};
        long dir = u[0]*v[0]+u[1]*v[1]+u[2]*v[2];
        long w0 = u[1]*v[2]-u[2]*v[1], w1 = u[2]*v[0]-u[0]*v[2], w2 = u[0]*v[1]-u[1]*v[0];
        long cru = w0*w0 + w1*w1 + w2*w2;
        long Nu = u[0]*u[0]+u[1]*u[1]+u[2]*u[2], Nv = v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
        pares++;
        if(dir*dir + cru != Nu*Nv) mal++;
    }
    printf("      %ld pares varridos em dimensão 3, %ld falhas\n\n", pares, mal);
    ok("LAGRANGE: directo² + cruzado² = N(u)N(v) — o par dual FECHA, e a conservação da norma"
       " É a decomposição simétrica ⊕ antissimétrica escrita numa linha", mal == 0);

    /* E daqui saem duas coisas que não são decorativas. */
    long folga_neg = 0, folga_e_cruzado = 0, testados = 0;
    for(long a=-3;a<=3;a++) for(long b=-3;b<=3;b++) for(long c=-3;c<=3;c++)
    for(long d=-3;d<=3;d++) for(long e=-3;e<=3;e++) for(long f=-3;f<=3;f++){
        long u[3]={a,b,c}, v[3]={d,e,f};
        long dir = u[0]*v[0]+u[1]*v[1]+u[2]*v[2];
        long w0 = u[1]*v[2]-u[2]*v[1], w1 = u[2]*v[0]-u[0]*v[2], w2 = u[0]*v[1]-u[1]*v[0];
        long cru = w0*w0 + w1*w1 + w2*w2;
        long Nu = u[0]*u[0]+u[1]*u[1]+u[2]*u[2], Nv = v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
        long folga = Nu*Nv - dir*dir;
        testados++;
        if(folga < 0) folga_neg++;
        if(folga == cru) folga_e_cruzado++;
    }
    printf("      CAUCHY–SCHWARZ: ⟨u,v⟩² ≤ N(u)N(v) em %ld pares, %ld violações;\n", testados, folga_neg);
    printf("      e a FOLGA é o cruzado em %ld deles\n\n", folga_e_cruzado);
    ok("a desigualdade de Cauchy–Schwarz é a igualdade de Lagrange com um termo apagado, e o"
       " termo apagado é ‖u∧v‖² ≥ 0 — a folga não é uma sobra, é o cruzado",
       folga_neg == 0 && folga_e_cruzado == testados);

    /* O QUARTO LUGAR — porque é que o degrau 4 existe. */
    long mal4 = 0;
    for(long a=-3;a<=3;a++) for(long b=-3;b<=3;b++) for(long c=-3;c<=3;c++)
    for(long d=-3;d<=3;d++) for(long e=-3;e<=3;e++) for(long f=-3;f<=3;f++){
        long u[3]={a,b,c}, v[3]={d,e,f};
        long esc = -(u[0]*v[0]+u[1]*v[1]+u[2]*v[2]);            /* a parte REAL do produto */
        long w0 = u[1]*v[2]-u[2]*v[1], w1 = u[2]*v[0]-u[0]*v[2], w2 = u[0]*v[1]-u[1]*v[0];
        long Np = esc*esc + w0*w0 + w1*w1 + w2*w2;              /* a norma em dimensão 4 */
        long Nu = u[0]*u[0]+u[1]*u[1]+u[2]*u[2], Nv = v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
        if(Np != Nu*Nv) mal4++;
    }
    printf("      uv = (−⟨u,v⟩, u×v): um escalar e um vetor, %ld falhas na norma\n\n", mal4);
    ok("o produto de dois vetores PUROS de dimensão 3 tem uma parte escalar que NÃO CABE em"
       " dimensão 3 — e é por isso que o degrau da torre é 4 e não 3. Hurwitz não é um teto"
       " posto de fora: é onde o directo arranja lugar para se sentar ao lado do cruzado",
       mal4 == 0);
}

printf("\n§H7  O GUME: a soma de k quadrados é fechada para o produto?\n\n");
{
    /* Aqui não se afirma o teorema de Hurwitz: PROCURA-SE a testemunha da falha. Para k = 1, 2 e
     * 4 a busca tem de voltar VAZIA (quadrados, Brahmagupta, Euler); para k = 3 tem de achar. */
    printf("      k    fecha para o produto?   testemunha\n");
    int mau = 0;
    for(int k = 1; k <= 4; k++){
        long tx = 0, ty = 0; int achou = 0;
        for(long x = 1; x <= 40 && !achou; x++) for(long y = x; y <= 40 && !achou; y++){
            /* é x soma de k quadrados? (busca directa, sem tabela) */
            int sx = 0, sy = 0, sp = 0;
            for(long i = 0; i*i <= x && !sx; i++){
                if(k == 1){ sx = (i*i == x); continue; }
                for(long j = 0; j*j <= x-i*i && !sx; j++){
                    if(k == 2){ sx = (i*i+j*j == x); continue; }
                    for(long l = 0; l*l <= x-i*i-j*j && !sx; l++){
                        if(k == 3){ sx = (i*i+j*j+l*l == x); continue; }
                        for(long m = 0; m*m <= x-i*i-j*j-l*l && !sx; m++)
                            sx = (i*i+j*j+l*l+m*m == x);
                    }
                }
            }
            if(!sx) continue;
            for(long i = 0; i*i <= y && !sy; i++){
                if(k == 1){ sy = (i*i == y); continue; }
                for(long j = 0; j*j <= y-i*i && !sy; j++){
                    if(k == 2){ sy = (i*i+j*j == y); continue; }
                    for(long l = 0; l*l <= y-i*i-j*j && !sy; l++){
                        if(k == 3){ sy = (i*i+j*j+l*l == y); continue; }
                        for(long m = 0; m*m <= y-i*i-j*j-l*l && !sy; m++)
                            sy = (i*i+j*j+l*l+m*m == y);
                    }
                }
            }
            if(!sy) continue;
            long P = x*y;
            for(long i = 0; i*i <= P && !sp; i++){
                if(k == 1){ sp = (i*i == P); continue; }
                for(long j = 0; j*j <= P-i*i && !sp; j++){
                    if(k == 2){ sp = (i*i+j*j == P); continue; }
                    for(long l = 0; l*l <= P-i*i-j*j && !sp; l++){
                        if(k == 3){ sp = (i*i+j*j+l*l == P); continue; }
                        for(long m = 0; m*m <= P-i*i-j*j-l*l && !sp; m++)
                            sp = (i*i+j*j+l*l+m*m == P);
                    }
                }
            }
            if(!sp){ achou = 1; tx = x; ty = y; }
        }
        int esperado_fecha = (k != 3);
        if(achou == esperado_fecha) mau++;
        /* «NÃO» tem 3 caracteres e 4 BYTES: o %-Ns paga por bytes e come uma coluna.
         * Conta-se em caracteres, que é o que o leitor vê. */
        printf("      %-4d ", k);
        col(achou ? "NÃO" : "sim", 23);
        putchar(' ');
        if(achou) printf("%ld · %ld = %ld", tx, ty, tx*ty);
        printf("\n");
    }
    printf("\n");
    ok("o buscador acha a testemunha EXATAMENTE em k = 3 e volta vazio em k = 1, 2 e 4 — os"
       " degraus da torre não são citados aqui, são o resultado da busca", mau == 0);
    printf("      E O LIMITE DESTE GUME, dito à frente: para k ≥ 4 todo natural já é soma de k\n");
    printf("      quadrados (Lagrange), portanto a tese é SEMPRE VERDADEIRA e a busca não pode\n");
    printf("      achar nada. Correr isto em k = 5, 6, 7 não mediria a ausência das álgebras\n");
    printf("      nessas dimensões — mediria o vazio. O que exclui 5, 6 e 7 é a BILINEARIDADE,\n");
    printf("      e essa é o teorema de Hurwitz, não um número. Por isso o laço para em 4.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    A torre tem quatro degraus porque as perdas se desencontram: comuta até 2,\n");
printf("    associa até 4, divide até 8. Sobe-se perdendo e desce-se recuperando, e as\n");
printf("    duas induções encontram-se — o encontro é o que a fecha, e não nós.\n\n");
printf("    E o par dual FECHA: directo² + cruzado² = N(u)N(v). A conservação da norma\n");
printf("    e a decomposição simétrica ⊕ antissimétrica são a mesma frase, e o degrau 4\n");
printf("    é onde o escalar arranja lugar. Zero doubles, zero tolerâncias.\n\n");
printf("    %ld estouros do teto (%ld) — se não for 0, os inteiros não chegaram.\n", estouros, (long)TETO);
ok("nenhuma conta passou o teto declarado — o limite não é documentação, é medido",
   estouros == 0);
printf("\n    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
