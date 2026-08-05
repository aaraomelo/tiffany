/* natural.c — A RÉGUA NÃO TRANSPORTA, E A VOLTA TRANSPORTA. O DISCRIMINANTE É λ².
 *
 * O Aarao, sobre As Noures de Ubaldi: "dissipar o corpo para ver seu lado dual só diz sobre
 * voce mesmo, nao é possivel que essa experiencia entre em correntes de pensamento pq nao é
 * verificavel por mais ninguem pq cada um tem sua regua particular e nenhuma regua de um corpo
 * serve em outro — pode ate virar teorema."
 *
 * Vira. E ja' estava meio escrito no texto: o thm:espectro diz que dois corpos duais diferem
 * exatamente em REGUA e DINAMICA. Faltava a consequencia, e ela e' a que separa o que este
 * projeto faz do que aquele livro fez:
 *
 *      J : V -> V*     (a REGUA)         NAO e' natural — nenhuma escolha dela transporta
 *      ev: V -> V**    (a VOLTA)         E' natural — transporta sempre, sem escolha
 *
 * A RAZAO E' O NUMERO LIQUIDO DE TRAVESSIAS. Sob a homotetia f = λ·id:
 *
 *      J  tem DOIS indices em V*, no MESMO sentido   ->  acumulam:  λ²·J
 *      ev tem um em V* e um em V**, em sentidos OPOSTOS -> cancelam: λ⁰·ev
 *
 * Logo a regua so' sobrevive quando λ²=1, isto e' λ=±1 — QUE E' A ESTACA. Nao e' coincidencia
 * com o σσ'=-1 do fim do sec:espectro: e' a mesma equacao. As unicas simetrias que uma regua
 * tolera sao involucoes, e e' POR ISSO que o espectro e' uma orbita da involucao.
 *
 * E o que isto diz de uma medida feita por dentro: quem mede o proprio corpo com o proprio
 * corpo produz um numero solidario com a escolha que o produziu. Nao e' falso. E' que nao ha'
 * transporte — nao existe morfismo que o leve a outro corpo e o mantenha. A medida pela volta
 * (bidual) transporta, e e' a unica que transporta.
 *
 *   §N1  a naturalidade de ev, por DOIS CAMINHOS que tem de concordar — e o controlo negativo
 *   §N2  a regua sob a homotetia: o residuo e' (λ²-1)·J, e a tabela mostra o expoente 2
 *   §N3  e nao e' so' a homotetia: contam-se os f de uma caixa que preservam J
 *   §N4  O NUMERO QUE FECHA: na MESMA caixa, quantos transportam ev e quantos transportam J
 *   §N5  a hipotese e' essencial: com J degenerada tudo passa — e' o isomorfismo que quebra
 *   §N6  λ²=1 da' exatamente {-1,+1}: o conjunto admissivel E' a estaca
 *
 * Tudo em inteiros exatos: residuo 0 ou falha, nunca "pequeno".
 *
 *   cc -O2 -std=c99 -Wall natural.c -o natural && ./natural
 */
#include <stdio.h>
#include "unidade.h"

typedef long long L;

/* ─── matrizes n×n inteiras, n<=3, em vetor de 9 ─────────────────────────────────── */
#define NMAX 3

static void mat_mul(int n, const L *A, const L *B, L *C){
    for(int i=0;i<n;i++) for(int j=0;j<n;j++){
        L s = 0;
        for(int k=0;k<n;k++) s += A[i*n+k]*B[k*n+j];
        C[i*n+j] = s;
    }
}
static void mat_transp(int n, const L *A, L *T){
    for(int i=0;i<n;i++) for(int j=0;j<n;j++) T[j*n+i] = A[i*n+j];
}
/* o puxar-para-tras da regua: f* ∘ J ∘ f, que em matrizes e' Aᵀ J A */
static void puxa(int n, const L *A, const L *J, L *R){
    L T[NMAX*NMAX], M[NMAX*NMAX];
    mat_transp(n, A, T);
    mat_mul(n, T, J, M);
    mat_mul(n, M, A, R);
}
/* residuo da regua: (Aᵀ J A) - J, entrada a entrada, somado em modulo */
static L residuo_regua(int n, const L *A, const L *J){
    L R[NMAX*NMAX]; puxa(n, A, J, R);
    L s = 0;
    for(int i=0;i<n*n;i++){ L d = R[i] - J[i]; s += d<0 ? -d : d; }
    return s;
}

int main(void)
{
    puts("A RÉGUA NÃO TRANSPORTA, E A VOLTA TRANSPORTA — o discriminante é λ²");
    puts("=====================================================================");
    puts("  J : V -> V*   é a régua (thm:espectro).   ev: V -> V** é a volta.");
    puts("  Transportar = o quadrado fechar para TODO morfismo f: V -> W.");
    puts("");

    /* ══════════════════════════════════════════════════════════════════════════════
     * §N1  A NATURALIDADE DE ev, POR DOIS CAMINHOS
     *
     * ev_V(x)(φ) = φ(x). A naturalidade pede  f**∘ev_V = ev_W∘f,  que avaliada em
     * (x, φ) é a igualdade de dois cálculos DIFERENTES:
     *
     *    caminho A (empurrar x, medir lá):    φ(Ax)      = Σ_i φ_i (Ax)_i
     *    caminho B (puxar φ, medir cá):       (Aᵀφ)(x)   = Σ_j (Aᵀφ)_j x_j
     *
     * Não é tautologia: são duas ordens de multiplicação, e discordam de imediato se
     * o dual de f for definido com A em vez de Aᵀ — que é o controlo negativo.
     * ═══════════════════════════════════════════════════════════════════════════════ */
    puts("§N1  ev É NATURAL — os dois caminhos, em inteiros exatos");
    puts("");
    {
        const int n = 3;
        L mal = 0, casos = 0, mal_errado = 0;
        /* varre matrizes A com entradas em {-2..2} em 4 posições e o resto fixo,
         * e vetores x, φ de uma família determinista — sem aleatório, sem memória */
        for(int a=-2;a<=2;a++) for(int b=-2;b<=2;b++)
        for(int c=-2;c<=2;c++) for(int d=-2;d<=2;d++){
            L A[9] = { a, b, 1,
                       c, d, -1,
                       1, -1, 2 };
            for(int t=0;t<5;t++){
                L x[3] = { t-2, 1-t, t*t-3 };
                L f[3] = { 2-t, t+1, 1-2*t };   /* φ, em coordenadas */

                /* caminho A: empurrar x por A, depois medir com φ em W */
                L Ax[3];
                for(int i=0;i<n;i++){ L s=0; for(int k=0;k<n;k++) s += A[i*n+k]*x[k]; Ax[i]=s; }
                L ladoA = 0; for(int i=0;i<n;i++) ladoA += f[i]*Ax[i];

                /* caminho B: puxar φ por Aᵀ (que é f*), depois medir com x em V */
                L Atf[3];
                for(int j=0;j<n;j++){ L s=0; for(int i=0;i<n;i++) s += A[i*n+j]*f[i]; Atf[j]=s; }
                L ladoB = 0; for(int j=0;j<n;j++) ladoB += Atf[j]*x[j];

                if(ladoA != ladoB) mal++;

                /* CONTROLO NEGATIVO: o dual definido com A em vez de Aᵀ */
                L Af[3];
                for(int i=0;i<n;i++){ L s=0; for(int k=0;k<n;k++) s += A[i*n+k]*f[k]; Af[i]=s; }
                L ladoErrado = 0; for(int j=0;j<n;j++) ladoErrado += Af[j]*x[j];
                if(ladoErrado != ladoA) mal_errado++;

                casos++;
            }
        }
        printf("      %lld pares (f, x, φ) percorridos, sem escolher nada\n", casos);
        printf("      caminho A ≠ caminho B em .......... %lld casos\n", mal);
        printf("      com o dual trocado (A em vez de Aᵀ)  %lld casos discordam\n", mal_errado);
        printf("\n");
        ok("ev é natural: empurrar-e-medir = puxar-e-medir, resíduo 0 exato em todos os casos",
           mal == 0);
        ok("e a asserção PODE falhar: trocar Aᵀ por A quebra-a na maioria dos casos",
           mal_errado > casos/2);
        puts("      Nenhuma escolha entrou nesta secção. É essa a diferença toda.");
        puts("");
    }

    /* ══════════════════════════════════════════════════════════════════════════════
     * §N2  A RÉGUA SOB A HOMOTETIA: o resíduo é (λ²-1)·J
     * ═══════════════════════════════════════════════════════════════════════════════ */
    puts("§N2  A RÉGUA SOB f = λ·id — e o expoente é 2, não 1");
    puts("");
    {
        const int n = 3;
        L J[9] = { 2, 1, 0,
                   1, 3, 1,
                   0, 1, 2 };          /* simétrica, não-degenerada: uma régua legítima */
        L normaJ = 0; for(int i=0;i<9;i++) normaJ += J[i]<0 ? -J[i] : J[i];

        puts("        λ    resíduo da RÉGUA   (λ²-1)·‖J‖   resíduo da VOLTA");
        puts("        ---  -----------------  -----------  ----------------");
        int mal_regua = 0, mal_volta = 0, zeros = 0;
        for(L lam = -3; lam <= 3; lam++){
            L A[9] = {0}; for(int i=0;i<n;i++) A[i*n+i] = lam;
            L r = residuo_regua(n, A, J);
            L prev = (lam*lam - 1) * normaJ; if(prev < 0) prev = -prev;

            /* a volta, sob a mesma homotetia: os dois caminhos do §N1 com A = λ·id */
            L rv = 0;
            for(int t=0;t<5;t++){
                L x[3] = { t-2, 1-t, t*t-3 }, f[3] = { 2-t, t+1, 1-2*t };
                L ladoA = 0, ladoB = 0;
                for(int i=0;i<n;i++) ladoA += f[i]*(lam*x[i]);
                for(int j=0;j<n;j++) ladoB += (lam*f[j])*x[j];
                L d = ladoA - ladoB; rv += d<0 ? -d : d;
            }
            printf("        %3lld  %17lld  %11lld  %16lld\n", lam, r, prev, rv);
            if(r != prev) mal_regua++;
            if(rv != 0)   mal_volta++;
            if(r == 0)    zeros++;
        }
        printf("\n");
        ok("o resíduo da régua é exatamente (λ²-1)·‖J‖ — o expoente é 2, e some no quadrado",
           mal_regua == 0);
        ok("o resíduo da volta é 0 para TODO λ — os dois índices cancelam-se", mal_volta == 0);
        ok("a régua só fecha em dois valores de λ, e são ±1", zeros == 2);
        puts("      J tem dois índices no mesmo sentido e eles somam; ev tem um de cada");
        puts("      e eles cancelam. O expoente É o número líquido de travessias.");
        puts("");
    }

    /* ══════════════════════════════════════════════════════════════════════════════
     * §N3 e §N4  A CAIXA INTEIRA: quantos morfismos transportam cada coisa
     * ═══════════════════════════════════════════════════════════════════════════════ */
    puts("§N3  E NÃO É SÓ A HOMOTETIA — a caixa toda dos morfismos 2×2");
    puts("");
    L cx_total = 0, cx_ev = 0;
    L guarda_id = 0, guarda_hip = 0, guarda_outra = 0, guarda_deg = 0;
    {
        const int n = 2;
        L Jid [4] = { 1, 0, 0, 1 };    /* o círculo   */
        L Jhip[4] = { 1, 0, 0,-1 };    /* a hipérbole */
        L Jout[4] = { 2, 1, 1, 3 };    /* outra régua qualquer, simétrica não-degenerada */
        L Jdeg[4] = { 0, 0, 0, 0 };    /* DEGENERADA — o controlo do §N5 */

        for(int a=-2;a<=2;a++) for(int b=-2;b<=2;b++)
        for(int c=-2;c<=2;c++) for(int d=-2;d<=2;d++){
            L A[4] = { a, b, c, d };
            cx_total++;
            if(residuo_regua(n, A, Jid ) == 0) guarda_id++;
            if(residuo_regua(n, A, Jhip) == 0) guarda_hip++;
            if(residuo_regua(n, A, Jout) == 0) guarda_outra++;
            if(residuo_regua(n, A, Jdeg) == 0) guarda_deg++;

            /* a volta, no mesmo A: φ(Ax) = (Aᵀφ)(x), para uma família de (x,φ) */
            int falhou = 0;
            for(int t=0;t<5;t++){
                L x[2] = { t-2, 1-t }, f[2] = { 2-t, t+1 };
                L ladoA = 0, ladoB = 0;
                for(int i=0;i<n;i++){ L s=0; for(int k=0;k<n;k++) s += A[i*n+k]*x[k]; ladoA += f[i]*s; }
                for(int j=0;j<n;j++){ L s=0; for(int i=0;i<n;i++) s += A[i*n+j]*f[i]; ladoB += s*x[j]; }
                if(ladoA != ladoB) falhou = 1;
            }
            if(!falhou) cx_ev++;
        }
        printf("      caixa: todas as matrizes 2×2 com entradas em {-2..2} .... %lld\n", cx_total);
        printf("      preservam a régua J = diag(1,1)   (o círculo) .......... %lld\n", guarda_id);
        printf("      preservam a régua J = diag(1,-1)  (a hipérbole) ........ %lld\n", guarda_hip);
        printf("      preservam a régua J = [[2,1],[1,3]] (outra qualquer) ... %lld\n", guarda_outra);
        printf("\n");
        ok("nenhuma régua é preservada por toda a caixa — o transporte falha para quase todo f",
           guarda_id < cx_total && guarda_hip < cx_total && guarda_outra < cx_total);
        ok("e o grupo que a preserva DEPENDE da régua: réguas diferentes, contagens diferentes",
           !(guarda_id == guarda_hip && guarda_hip == guarda_outra));
        puts("");

        puts("§N4  O NÚMERO QUE FECHA — a mesma caixa, os dois transportes lado a lado");
        puts("");
        printf("        o que se transporta      quantos dos %lld morfismos o preservam\n", cx_total);
        printf("        -----------------------  ----------------------------------\n");
        printf("        a VOLTA  (ev: V->V**)    %lld   — todos\n", cx_ev);
        printf("        a RÉGUA  (J: V->V*)      %lld   — o grupo ortogonal de J, e mais nada\n", guarda_id);
        printf("\n");
        ok("a volta transporta por TODO morfismo da caixa, sem excepção", cx_ev == cx_total);

        /* O AARAO: "esses morfismos que voce encontrou, voce mesmo disse: e' morfismo, e' o
         * MESMO CORPO." Esta' certo, e muda o enunciado. Se AᵀJA = J entao A e' isometria, e
         * pelo thm:espectro (corpo = regua + dinamica) preservar a regua E' ser o mesmo corpo.
         * Logo os 8 nao sao "poucos que transportam": sao o GRUPO DE AUTOMORFISMOS — o lugar
         * onde nao se saiu. A prova de que e' grupo e' fechar por produto, identidade e inversa,
         * e e' o que se mede aqui. A regua nao transporta pouco: NAO TRANSPORTA. */
        {
            L Jid[4] = { 1,0,0,1 };
            int tem_id = 0, fora_produto = 0, sem_inversa = 0, pares = 0;
            for(int a=-2;a<=2;a++) for(int b=-2;b<=2;b++)
            for(int c=-2;c<=2;c++) for(int d=-2;d<=2;d++){
                L A[4] = { a,b,c,d };
                if(residuo_regua(2, A, Jid) != 0) continue;
                if(a==1 && b==0 && c==0 && d==1) tem_id = 1;
                /* inversa: para J = I a inversa e' a transposta, e tem de estar no conjunto */
                L At[4]; mat_transp(2, A, At);
                if(residuo_regua(2, At, Jid) != 0) sem_inversa++;
                for(int p=-2;p<=2;p++) for(int q=-2;q<=2;q++)
                for(int r=-2;r<=2;r++) for(int s=-2;s<=2;s++){
                    L B[4] = { p,q,r,s };
                    if(residuo_regua(2, B, Jid) != 0) continue;
                    L P[4]; mat_mul(2, A, B, P);
                    pares++;
                    if(residuo_regua(2, P, Jid) != 0) fora_produto++;
                }
            }
            printf("      e o conjunto que preserva a régua é FECHADO:\n");
            printf("        contém a identidade ................... %s\n", tem_id ? "sim" : "NÃO");
            printf("        %d produtos A·B testados, saíram fora .. %d\n", pares, fora_produto);
            printf("        elementos sem inversa no conjunto ..... %d\n", sem_inversa);
            printf("\n");
            ok("os que preservam a régua formam GRUPO: identidade, produto e inversa fecham",
               tem_id && fora_produto == 0 && sem_inversa == 0);
            ok("logo não são morfismos que transportam a régua para fora — são o próprio corpo",
               guarda_id > 0 && tem_id && fora_produto == 0);
        }
        puts("      É ISTO QUE FECHA O TEOREMA, e é mais forte do que 'transporta pouco':");
        puts("      o conjunto onde a régua transporta é o grupo dos automorfismos — o lugar");
        puts("      onde NÃO SE SAIU DO CORPO. Para fora dele a régua não vai de todo.");
        puts("      Uma medida pela volta chega a outro corpo. Uma medida pela régua só");
        puts("      'chega' quando o destino já era o mesmo corpo — e aí não viajou nada.");
        puts("");

        puts("§N5  A HIPÓTESE É ESSENCIAL — com a régua degenerada, tudo passa");
        puts("");
        printf("      preservam J = 0 (degenerada, não é isomorfismo) ........ %lld de %lld\n",
               guarda_deg, cx_total);
        printf("\n");
        ok("com J degenerada o quadrado fecha para todo f — logo é o ISOMORFISMO que quebra",
           guarda_deg == cx_total);
        puts("      A prova usa a hipótese e não é retórica: (λ²-1)J = 0 só força λ²=1");
        puts("      porque J é não-nula. Tire-se a régua e não há nada para não transportar.");
        puts("");
    }

    /* ══════════════════════════════════════════════════════════════════════════════
     * §N6  O CONJUNTO ADMISSÍVEL É A ESTACA
     * ═══════════════════════════════════════════════════════════════════════════════ */
    puts("§N6  λ² = 1 DÁ EXATAMENTE {-1,+1} — e isso é a estaca, não uma coincidência");
    puts("");
    {
        const int n = 3;
        L J[9] = { 2, 1, 0,
                   1, 3, 1,
                   0, 1, 2 };
        int achados = 0, neg = 0, pos = 0;
        for(L lam = -12; lam <= 12; lam++){
            L A[9] = {0}; for(int i=0;i<n;i++) A[i*n+i] = lam;
            if(residuo_regua(n, A, J) == 0){ achados++; if(lam<0) neg++; if(lam>0) pos++; }
        }
        printf("      varridos λ = -12 .. 12 ....... %d escalares admissíveis\n", achados);
        printf("      e são simétricos em torno de 0: %d negativo, %d positivo\n", neg, pos);
        printf("\n");
        ok("os únicos escalares que a régua tolera são ±1 — o par da involução", achados == 2);
        ok("e vêm em par, um de cada lado do zero: é σσ' = -1", neg == 1 && pos == 1);
        puts("      O fim do sec:espectro diz que o espectro é uma órbita da involução.");
        puts("      Esta é a razão: as únicas simetrias que uma régua admite são involuções,");
        puts("      e por isso mudar de cor é aplicar a estaca — não há outra maneira.");
        puts("");
    }

    /* ══════════════════════════════════════════════════════════════════════════════
     * §N7  NO CORPO DESTE TEXTO — e não num corpo qualquer que eu tenha trazido
     *
     * O AARAO: "voce esta colocando sua confusao no texto. O corpo e' uma classe de
     * racionais com gerador irracional, e' isso."
     *
     * Esta' certo, e a versao anterior desta seccao era o defeito com o nome dele: eu
     * tinha enunciado o teorema "sobre um anel de escalares que contenha algum λ com
     * λ²≠1" e fui medir em F_2 e F_3 — que NAO SAO corpos deste texto. Depois declarei
     * como fronteira honesta um buraco que so' existia por causa da regua que trouxe.
     * PARAMETRIZEI O QUE O TEXTO JA' TINHA FIXADO.
     *
     * O corpo e' Q(σ), σ o gerador metalico: σ² = mσ + 1. Nele a pergunta nao tem
     * fronteira nenhuma — tem RESPOSTA COMPLETA, e ela e' a estaca:
     *
     *     λ = a + bσ,  λ² = (a²+b²) + (2ab + m b²)σ
     *     λ² = 1  ⟺  a²+b² = 1  e  2ab + m b² = 0
     *              ⟺  (a,b) = (±1, 0)          [pois (0,±1) da' m ≠ 0]
     *
     * Duas solucoes, ±1, para TODO metal. Nao e' "existe algum λ que serve de
     * contra-exemplo": e' o conjunto inteiro das solucoes, e o resto do corpo — que e'
     * infinito — falha todo. A hipotese que eu tinha acrescentado era o preco da regua.
     * ═══════════════════════════════════════════════════════════════════════════════ */
    puts("§N7  NO CORPO DESTE TEXTO: Q(σ), σ² = mσ + 1 — e aí não há fronteira");
    puts("");
    {
        puts("        metal  σ²=mσ+1   λ=a+bσ com λ²=1, em a,b ∈ {-6..6}   quais");
        puts("        -----  --------  ---------------------------------  ------------");
        int mal = 0;
        for(int m = 1; m <= 5; m++){
            int achados = 0, so_mais_menos_um = 1;
            for(int a=-6;a<=6;a++) for(int b=-6;b<=6;b++){
                L r = (L)a*a + (L)b*b;              /* parte racional de λ² */
                L s = 2LL*a*b + (L)m*b*b;           /* parte em σ de λ²     */
                if(r == 1 && s == 0){
                    achados++;
                    if(!((a==1||a==-1) && b==0)) so_mais_menos_um = 0;
                }
            }
            printf("        %-5d  %-8s  %-33d  %s\n", m,
                   m==1?"ouro":m==2?"prata":m==3?"bronze":"—", achados,
                   so_mais_menos_um && achados==2 ? "±1, e mais nada" : "OUTROS");
            if(!(so_mais_menos_um && achados == 2)) mal++;
        }
        printf("\n");
        ok("em Q(σ) a equação λ²=1 tem exatamente duas soluções, ±1, para todo metal", mal == 0);
        ok("logo a estaca não é hipótese acrescentada: é o conjunto-solução COMPLETO no corpo",
           mal == 0);
        puts("      Q(σ) é corpo, logo (λ-1)(λ+1)=0 força λ=±1 sem mais nada; a varredura");
        puts("      acima é a verificação em Z[σ], onde a conta fecha em inteiros exatos.");
        puts("      E o corpo é infinito: todo o resto dele falha o transporte da régua.");
        puts("");
    }

    puts("=====================================================================");
    puts("  [~] a régua é o corpo: medir por dentro dá um número solidário com a escolha.");
    puts("  [~] a volta é canónica: é a única medida que chega a outro corpo inteira.");
    puts("  [~] e o que separa as duas é o expoente de λ — duas travessias no mesmo");
    puts("      sentido acumulam, uma em cada sentido cancela.");
    printf("\n  %d unidade(s), %d falha(s) — RESÍDUO %s\n",
           unidades, falhas, falhas ? "NÃO NULO" : "0");
    return falhas ? 1 : 0;
}
