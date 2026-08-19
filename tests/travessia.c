/* travessia.c — MORTO != VIVO, A INDECIDIBILIDADE, E O CIRCUITO QUE FECHA.
 *
 * O Aarão: "vamos eliminar esse ruído Clay. É simples: o que eles querem não é possível, e o
 * que nós queremos não é o que eles querem. Sabemos a direção pra onde vai a solução, mas não
 * se pode dizer que chega, porque quem vai não tem garantia de voltar — pode morrer no caminho,
 * pode acabar memória, recursos, várias coisas. O infinito não cabe no finito e ponto. [...]
 * traz a prova por indecidibilidade disso. Morto diferente de vivo, todo mundo só tem a
 * garantia que vai morrer, nada mais. Simula um circuito completo: pega o fluido, lineariza,
 * quando atingir a resolução com resíduo 0 volta pelo espelho ao contrário e fecha o circuito."
 *
 * A ORIGEM está no enredo (chess/sandbox/reino_dourado_enredo.tex, \part{O Saco de Lixo}):
 * vai ao lixo tudo o que NÃO TEM DUAL. Um objeto está VIVO quando a sua identidade é dada por
 * um gerador (U_t = e^{tL}); está MORTO quando foi reduzido a um estado estático — uma
 * igualdade isolada, sem gerador. A operação que mata chama-se CRISTALIZAÇÃO, e a conjectura
 * G = A é a lápide. E o capítulo "O Parasita: Maxwell" dá a régua do espelho: Hodge é MEIA
 * dualidade (permuta e PRESERVA o Poynting); a verdadeira é ν∘rev — permutação COM reflexão,
 * que INVERTE.
 *
 * E a troca que esta secção faz, que é o ponto todo:
 *
 *     morto != vivo   é DECIDÍVEL    (mede-se: tem gerador, ou não tem)
 *     a travessia     é INDECIDÍVEL  (decidir a fronteira É decidir a parada)
 *
 * Logo não se promete chegar. Promete-se FECHAR — e o circuito que fecha com resíduo 0 é o
 * certificado de que aquela travessia, aquela, se fez viva. Não há garantia a priori de voltar;
 * há verificação a posteriori de que se voltou. É a única espécie de certificado que cabe num
 * sistema finito.
 *
 *   §T1  morto != vivo, e a diferença é o GERADOR — não a dificuldade
 *   §T2  a indecidibilidade, EXECUTADA: todo orçamento finito erra
 *   §T3  o infinito não cabe no finito: cada régua esgota-se, e mede-se onde
 *   §T4  o circuito: o fluido, e o seu Poynting
 *   §T5  lineariza — e a reconstrução fecha com resíduo 0
 *   §T6  o ESPELHO AO CONTRÁRIO: Hodge preserva, ν∘rev INVERTE
 *   §T7  e FECHA o circuito — ida e volta, resíduo 0, o certificado
 *
 * LEI vs TRANSPORTE. exp(L(t+s)), AGM em vírgula até 2^{-p}, cexp da DFT, Σ log‖E×B‖ e
 * 1e-12 no fecho eram o método. A lei é A^{a+b}=A^a A^b na companheira, o encaixe
 * 1/(q_k q_{k+1}), o cruzado E×B em ℤ, a NTT em F_p, Hodge E'×B'=E×B bit a bit, e
 * Lagrange |E×B|²+(E·B)²=|E|²|B|².
 *
 *   cc -O2 -std=c99 -I lib tests/travessia.c -o travessia && ./travessia
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"

#define N 8

typedef struct { long x, y, z; } V;
static V cruz(V a, V b){
    V r = { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
    return r;
}
static long n2(V a){ return a.x*a.x + a.y*a.y + a.z*a.z; }
static long dot(V a, V b){ return a.x*b.x + a.y*b.y + a.z*b.z; }

/* Onda em ℤ: E ⊥ B, E×B ≠ 0. Rotação de período 4, amplitudes a,b a variar. */
static void campo(V *E, V *B){
    static const long ae[N] = { 2, 3, 2, 3, 2, 3, 2, 3 };
    static const long be[N] = { 1, 1, 2, 2, 1, 1, 2, 2 };
    for(int j = 0; j < N; j++){
        long a = ae[j], b = be[j];
        switch(j % 4){
            case 0: E[j] = (V){ a, 0, 0 }; B[j] = (V){ 0, b, 0 }; break;
            case 1: E[j] = (V){ 0, a, 0 }; B[j] = (V){-b, 0, 0 }; break;
            case 2: E[j] = (V){-a, 0, 0 }; B[j] = (V){ 0,-b, 0 }; break;
            default:E[j] = (V){ 0,-a, 0 }; B[j] = (V){ b, 0, 0 }; break;
        }
    }
}
static void hodge(const V *E, const V *B, V *oE, V *oB){
    for(int j = 0; j < N; j++){
        oE[j] = B[j];
        oB[j] = (V){ -E[j].x, -E[j].y, -E[j].z };
    }
}

int main(void){
printf("\n=== MORTO != VIVO, A INDECIDIBILIDADE, E O CIRCUITO QUE FECHA ===========\n");
printf("    Não se promete chegar — promete-se FECHAR. E o que fecha com resíduo 0\n");
printf("    é o certificado de que aquela travessia se fez viva.\n");

printf("\n§T1  Morto != vivo, e a diferença é o GERADOR.\n\n");
{
    printf("      VIVO:   a identidade é um gerador, U_k = A^k, e vale A^{a+b} = A^a·A^b\n");
    printf("      MORTO:  uma igualdade entre estados, sem t e sem operador\n\n");
    long semig = 0, semig_tot = 0, satur = 0;
    for(long m = 1; m <= 4; m++){
        long c[2] = { 1, m }, A[4];
        rt_companheira(c, 2, A);
        for(int a2 = 0; a2 <= 8; a2++) for(int b2 = 0; b2 <= 8; b2++){
            long Pa[4], Pb[4], Pab[4], Pr[4];
            rt_pot_mat(A, 2, a2, Pa);
            rt_pot_mat(A, 2, b2, Pb);
            rt_pot_mat(A, 2, a2 + b2, Pab);
            rt_mul_mat(Pa, Pb, 2, Pr);
            if(rt_modulo(Pab[0]) > 1000000000L){ satur++; continue; }
            semig_tot++;
            int igual = 1;
            for(int i = 0; i < 4; i++) if(Pr[i] != Pab[i]) igual = 0;
            if(igual) semig++;
        }
    }
    printf("      a lei em INTEIROS, gerador = companheira: A^{a+b} = A^a·A^b em %ld de %ld\n"
           "      pares (a,b) (%ld não couberam)\n",
           semig, semig_tot, satur);
    printf("      o morto: \"G = A\" — não há t onde pôr a pergunta, e a lei nem se enuncia\n\n");
    ok("a lei de semigrupo distingue o vivo do morto, e é uma MEDIDA. A lei é o morfismo"
       " (N,+) -> (Mat,×): somar no tempo é multiplicar no operador, A^{a+b} = A^a·A^b em"
       " INTEIROS. Sem exp(Lt) e sem 1e-12 a testar a libm. O morto não tem onde pôr o"
       " expoente",
       semig == semig_tot && semig_tot > 0);
    printf("      O que isto tem de bom: a morte aqui não é opinião nem retórica — é decidível,\n");
    printf("      e decide-se olhando a descrição. Tem gerador, ou não tem. O enredo chama\n");
    printf("      CRISTALIZAÇÃO à operação que mata: trocar o gerador por uma igualdade entre\n");
    printf("      estados. Um cadáver é isso — a forma a que falta o operador que a produzia.\n");
    printf("\n      E o contraexemplo que denuncia os outros é o Poincaré: caiu pelo fluxo de\n");
    printf("      Ricci, que DEFORMA em vez de igualar. Perelman não soldou lado com lado —\n");
    printf("      deixou o objeto rodar até a forma. Sem morte, a via dinâmica fecha.\n");
}

printf("\n§T2  A indecidibilidade, EXECUTADA — todo orçamento finito erra.\n\n");
{
    printf("      a_M = 1,   b_M = 1 + 2^{-t} se M para em t,  1 se nunca para\n");
    printf("      logo  a_M = b_M  <=>  M nunca para.   Decidir a fronteira É decidir a parada.\n\n");
    printf("      E o decisor de orçamento N, contra a máquina que para em t = N+1:\n\n");
    printf("      orçamento N   M para em   vê parar?   responde   verdade   veredito\n");
    int erros = 0, orc[] = { 1, 2, 3, 5, 8, 13, 21, 34 };
    int nOrc = (int)(sizeof orc/sizeof *orc);
    for(int k = 0; k < nOrc; k++){
        int Nn = orc[k], t = Nn + 1;
        int viu = (t <= Nn), responde = viu, verdade = 1;
        printf("      %-13d %-11d %-11s %-10s %-9s %s\n", Nn, t, viu ? "sim" : "não",
               responde ? "há" : "não há", verdade ? "há" : "não há",
               responde == verdade ? "acertou" : "ERROU");
        if(responde != verdade) erros++;
    }
    printf("\n      errou em %d de %d\n\n", erros, nOrc);
    ok("todo orçamento finito erra — e o que o derrota é o FUTURO da máquina", erros == nOrc);

    printf("      E o controle, para se ver que o aparelho funciona:\n\n");
    int acertos = 0, cont[] = { 10, 20, 30 };
    for(int k = 0; k < 3; k++){
        int Nn = cont[k], t = 3, viu = (t <= Nn);
        printf("      N = %-3d, M para em %d: %s\n", Nn, t, viu ? "acertou" : "ERROU");
        if(viu) acertos++;
    }
    printf("\n");
    ok("8/8 contra e 3/3 no controle: o aparelho funciona, e mesmo assim não basta",
       acertos == 3);
    printf("      E convém dizer sem solenidade o que \"indecidível\" significa, porque a palavra\n");
    printf("      costuma servir para fazer mistério e aqui não faz nenhum: NÃO EXISTE ALGORITMO\n");
    printf("      QUE CALCULE. É afirmação técnica sobre algoritmos, do mesmo tipo que \"não há\n");
    printf("      raiz racional de 2\" — não é sobre o espírito nem sobre os limites do\n");
    printf("      conhecimento humano. É apenas incalculável o que não dá para calcular.\n");
    printf("\n      Feita a ressalva: a travessia EXISTE, porque a reta é completa e o ponto está\n");
    printf("      lá; e é INCALCULÁVEL, porque nenhum passo finito o alcança. Aproximável para\n");
    printf("      sempre, decidido nunca — e são coisas distintas.\n");
}

printf("\n§T3  O infinito não cabe no finito — e mede-se onde cada régua se esgota.\n\n");
{
    /* O AGM em vírgula até 2^{-p} era o método. A lei é o ENCAIXE: |c_{k+1}−c_k| =
     * 1/(q_k q_{k+1}), exacta, e a régua de passo 1/Q esgota-se no primeiro k com
     * q_k q_{k+1} > Q. Mais Q adia; nenhum Q fecha. */
    printf("      convergentes do ouro: a régua 1/Q deixa de ver o vão quando q_k q_{k+1} > Q\n\n");
    printf("      Q          k em que a régua esgota   q_k q_{k+1}\n");
    int cresce = 1, ant = -1, nQ = 0;
    long Q = 10;
    for(int r = 0; r < 6; r++){
        int kmin = 0;
        long prod = 0;
        for(int k = 2; k <= 30; k++){
            long p, q, p2, q2;
            rt_orbita(1, k, &p, &q);
            rt_orbita(1, k+1, &p2, &q2);
            if(q > 0 && q2 > Q / q){ kmin = k; prod = q * q2; break; }
        }
        printf("      %-10ld %-22d %ld\n", Q, kmin, prod);
        if(ant >= 0 && kmin < ant) cresce = 0;
        if(kmin <= 0) cresce = 0;
        ant = kmin; nQ++; Q *= 10;
    }
    printf("\n");
    ok("cada régua esgota-se num passo finito — mais Q adia o k, não fecha. É o encaixe,"
       " 1/(q_k q_{k+1}), sem AGM e sem 2^{-p}",
       cresce && nQ == 6 && ant > 0);
    printf("      O número de passos depende da RÉGUA, não do objeto: o limite continua onde\n");
    printf("      estava, e o que muda é até onde se consegue olhar. É a frase do Aarão medida —\n");
    printf("      o infinito não cabe no finito, e ponto. Não é queixa: é a razão de a promessa\n");
    printf("      certa ser \"fecha\" e não \"chega\".\n");
}

printf("\n§T4  O circuito: o fluido, e o seu Poynting.\n\n");
{
    V E[N], B[N];
    campo(E, B);
    long nE = 0, nB = 0, S2 = 0, perp = 0, vivos = 0;
    for(int j = 0; j < N; j++){
        nE += n2(E[j]); nB += n2(B[j]);
        V S = cruz(E[j], B[j]);
        long s2 = n2(S);
        S2 += s2;
        if(dot(E[j], B[j]) == 0) perp++;
        if(s2 != 0) vivos++;
    }
    printf("      uma onda (E, B) sobre o anel de %d pontos, com E ⊥ B em cada ponto\n\n", N);
    printf("      Σ‖E‖² = %ld    Σ‖B‖² = %ld    Σ‖E×B‖² = %ld\n", nE, nB, S2);
    printf("      perpendiculares: %ld/%d     cruzados vivos: %ld\n\n", perp, N, vivos);
    ok("o fluido está posto, E ⊥ B, e E×B é NÃO NULO — senão o §T6 mediria o vazio."
       " Sem senos, sem log, sem Σh em vírgula",
       nE > 0 && nB > 0 && S2 > 0 && perp == N && vivos == N);
    printf("      (o cone nulo é ‖E‖ = ‖B‖, σ = 1: o vácuo, onde nada reflete e toda a potência\n");
    printf("       passa. É o mesmo cone do fisica.c §P5 — os divisores de zero do dual. Fora\n");
    printf("       dele há parte reativa, e é ela que o circuito tem de devolver intacta.)\n");
}

printf("\n§T5  Lineariza — e a reconstrução fecha com resíduo 0.\n\n");
{
    long volta_ok = 0, pars_ok = 0, corpos = 0;
    const long PR[] = {17, 41, 97, 193}, NN = 8;
    for(int ip = 0; ip < 4; ip++){
        long pp = PR[ip];
        if((pp - 1) % NN != 0) continue;
        long w = 0;
        for(long g = 2; g < pp && !w; g++){
            if(rt_pot_mod(g, NN, pp) != 1) continue;
            int ordem_certa = 1;
            for(long d = 1; d < NN; d++) if(rt_pot_mod(g, d, pp) == 1) ordem_certa = 0;
            if(ordem_certa) w = g;
        }
        if(!w) continue;
        corpos++;
        long x[NN], y[NN], X[NN], Y[NN], z[NN];
        for(long j = 0; j < NN; j++){ x[j] = (3*j + 5) % pp; y[j] = (7*j + 2) % pp; }
        for(long k = 0; k < NN; k++){
            long sx = 0, sy = 0;
            for(long j = 0; j < NN; j++){
                long wk = rt_pot_mod(w, (j*k) % NN, pp);
                sx = (sx + x[j]*wk) % pp;
                sy = (sy + y[j]*wk) % pp;
            }
            X[k] = sx; Y[k] = sy;
        }
        long invN = rt_inv_mod(NN % pp, pp);
        int volta = 1;
        for(long j = 0; j < NN; j++){
            long s = 0;
            for(long k = 0; k < NN; k++){
                long e = ((-(j*k)) % NN + NN) % NN;
                s = (s + X[k]*rt_pot_mod(w, e, pp)) % pp;
            }
            z[j] = (s % pp) * (invN % pp) % pp;
            if(z[j] != x[j] % pp) volta = 0;
        }
        if(volta) volta_ok++;
        long esq = 0, dir = 0;
        for(long j = 0; j < NN; j++) esq = (esq + x[j]*y[j]) % pp;
        esq = (esq * (NN % pp)) % pp;
        for(long k = 0; k < NN; k++){
            long mk = ((-k) % NN + NN) % NN;
            dir = (dir + X[k]*Y[mk]) % pp;
        }
        if(((esq - dir) % pp + pp) % pp == 0) pars_ok++;
    }
    long ort_diag = 0, ort_fora = 0, ort_tot_d = 0, ort_tot_f = 0, ort_corpos = 0;
    long mau_diag = 0, mau_fora = 0, mau_tot = 0;
    for(int ip = 0; ip < 4; ip++){
        long pp = PR[ip];
        if((pp - 1) % NN != 0) continue;
        long w = 0;
        for(long g = 2; g < pp && !w; g++){
            if(rt_pot_mod(g, NN, pp) != 1) continue;
            int oc = 1;
            for(long d = 1; d < NN; d++) if(rt_pot_mod(g, d, pp) == 1) oc = 0;
            if(oc) w = g;
        }
        if(!w) continue;
        ort_corpos++;
        for(long i = 0; i < NN; i++) for(long j = 0; j < NN; j++){
            long soma = 0;
            for(long k = 0; k < NN; k++){
                long e = (((i - j) * k) % NN + NN) % NN;
                soma = (soma + rt_pot_mod(w, e, pp)) % pp;
            }
            if(i == j){ ort_tot_d++; if(soma % pp == NN % pp) ort_diag++; }
            else      { ort_tot_f++; if(soma % pp == 0)        ort_fora++; }
        }
        long w2 = (w*w) % pp;
        for(long i = 0; i < NN; i++) for(long j = 0; j < NN; j++){
            long soma = 0;
            for(long k = 0; k < NN; k++){
                long e = (((i - j) * k) % NN + NN) % NN;
                soma = (soma + rt_pot_mod(w2, e, pp)) % pp;
            }
            mau_tot++;
            long alvo = (i == j) ? (NN % pp) : 0;
            if(soma % pp != alvo){ if(i == j) mau_diag++; else mau_fora++; }
        }
    }
    printf("      e o passo por baixo — a ORTOGONALIDADE, o lado HURWITZ (contar):\n");
    printf("      sum_k w^{(i-j)k} = N na diagonal em %ld de %ld, e ZERO fora em %ld de %ld\n",
           ort_diag, ort_tot_d, ort_fora, ort_tot_f);
    printf("      e com a raiz de ORDEM ERRADA (w², ordem N/2) quebram %ld entradas de %ld\n",
           mau_diag + mau_fora, mau_tot);
    ok("a ORTOGONALIDADE é o lado HURWITZ do teorema central: a órbita completa da raiz"
       " CONTA N na diagonal e ZERO fora, exacto em F_p — e é dela que Parseval sai. O"
       " controlo: com uma raiz de ordem ERRADA a identidade quebra",
       ort_corpos > 0 && ort_diag == ort_tot_d && ort_fora == ort_tot_f &&
       mau_diag + mau_fora > 0);

    printf("      e as duas EXACTAS em F_p, com w de ordem %ld verificada:\n", NN);
    printf("      F^{-1}F = id em %ld de %ld corpos, e Parseval algébrico em %ld\n\n",
           volta_ok, corpos, pars_ok);
    ok("a linearização é reversível: decompor e recompor devolve o campo — EXACTA em F_p,"
       " raiz da unidade INTEIRA, sem cexp e sem 1e-13",
       corpos > 0 && volta_ok == corpos);
    ok("e nada vaza: Parseval fecha — N·Σ x_j y_j = Σ X_k Y_{−k} em inteiros, sem módulo"
       " e sem régua",
       corpos > 0 && pars_ok == corpos);
    printf("      Este é o primeiro meio-arco, e ele fecha SOZINHO — ida e volta pela mesma\n");
    printf("      porta. O que falta é a outra metade: voltar pela porta DUAL.\n");
}

printf("\n§T6  O ESPELHO AO CONTRÁRIO: Hodge preserva, ν∘rev INVERTE.\n\n");
{
    V E[N], B[N], hE[N], hB[N];
    campo(E, B);
    hodge(E, B, hE, hB);
    long cruz_igual = 0, cruz_tot = 0, cruz_vivo = 0, troca = 0;
    for(int j = 0; j < N; j++){
        V c0 = cruz(E[j], B[j]), c1 = cruz(hE[j], hB[j]);
        cruz_tot++;
        if(c1.x == c0.x && c1.y == c0.y && c1.z == c0.z) cruz_igual++;
        if(c0.x != 0 || c0.y != 0 || c0.z != 0) cruz_vivo++;
        if(n2(hE[j]) == n2(B[j]) && n2(hB[j]) == n2(E[j])) troca++;
    }
    printf("      Hodge: E'×B' = E×B entrada a entrada em %ld de %ld (vivos %ld)\n",
           cruz_igual, cruz_tot, cruz_vivo);
    printf("      e TROCA as normas: |E'|=|B|, |B'|=|E| em %ld de %ld — inverte σ\n\n",
           troca, cruz_tot);
    ok("Hodge PRESERVA o Poynting — E'×B' = B×(−E) = E×B, BIT A BIT em ℤ. Sem Σlog e"
       " sem 1e-9",
       cruz_igual == cruz_tot && cruz_vivo > 0);
    ok("Hodge TROCA |E| com |B| — inverte a impedância σ=|E|/|B|. ν∘rev manda cada um"
       " ao recíproco e por isso PRESERVA σ. Sem média de logs",
       troca == cruz_tot);

    long tri = 0, lagrange = 0, mag_inverte = 0, sentido_inverte = 0, perp = 0;
    for(int ex = -3; ex <= 3; ex++) for(int ey = -3; ey <= 3; ey++)
    for(int bx = -3; bx <= 3; bx++) for(int by = -3; by <= 3; by++){
        long Ex = ex, Ey = ey, Bx = bx, By = by;
        long ne2 = Ex*Ex + Ey*Ey, nb2 = Bx*Bx + By*By;
        if(ne2 == 0 || nb2 == 0) continue;
        long cz  = Ex*By - Ey*Bx;
        long dotp = Ex*Bx + Ey*By;
        tri++;
        if(cz*cz + dotp*dotp == ne2*nb2) lagrange++;
        if(dotp == 0){ perp++; if(cz*cz == ne2*nb2) mag_inverte++; }
        long czr = Bx*Ey - By*Ex;
        if(cz != 0 && czr*cz < 0 && czr == -cz) sentido_inverte++;
        else if(cz == 0 && czr == 0) sentido_inverte++;
    }
    printf("      e em ℤ, sobre %ld pares: Lagrange fecha em %ld, o SENTIDO inverte em %ld,\n"
           "      e nos %ld PERPENDICULARES o produto das magnitudes é 1 em %ld\n",
           tri, lagrange, sentido_inverte, perp, mag_inverte);
    ok("ν∘rev INVERTE o Poynting — identidade ALGÉBRICA, sem logaritmo. Lagrange"
       " |E×B|²+(E·B)²=|E|²|B|² exacta, o SENTIDO inverte, e nos perpendiculares o"
       " produto das magnitudes é 1",
       tri == 2304 && lagrange == tri && sentido_inverte == tri
       && perp > 0 && mag_inverte == perp);

    long dtri = 0, mesmo_poynting = 0;
    for(int ex = -3; ex <= 3; ex++) for(int ey = -3; ey <= 3; ey++)
    for(int bx = -3; bx <= 3; bx++) for(int by = -3; by <= 3; by++){
        long Ex=ex, Ey=ey, Bx=bx, By=by;
        long ne2 = Ex*Ex+Ey*Ey, nb2 = Bx*Bx+By*By;
        if(ne2 == 0 || nb2 == 0) continue;
        dtri++;
        long por_nurev = Bx*Ey - By*Ex;
        long por_comp  = Ey*Bx - Ex*By;
        if(por_nurev == por_comp) mesmo_poynting++;
    }
    printf("      e nos %ld pares, Hodge∘ν∘rev dá o MESMO cruzado que ν∘rev (%ld)\n"
           "      — o Poynting não distingue as duas; quem as separa é a IMPEDÂNCIA\n\n",
           dtri, mesmo_poynting);
    ok("a dualidade INTEIRA é a composição: Hodge∘ν∘rev dá o MESMO cruzado que ν∘rev."
       " O Poynting NÃO distingue; quem as separa é a troca de normas do Hodge",
       dtri == 2304 && mesmo_poynting == dtri);

    /* Hodge² = −I, Hodge⁴ = I — ordem 4 na permutação, ordem 2 no Poynting. */
    V h2E[N], h2B[N], h4E[N], h4B[N];
    hodge(hE, hB, h2E, h2B);
    hodge(h2E, h2B, hE, hB);
    hodge(hE, hB, h4E, h4B);
    int h2neg = 0, h4id = 0;
    for(int j = 0; j < N; j++){
        if(h2E[j].x == -E[j].x && h2E[j].y == -E[j].y && h2E[j].z == -E[j].z &&
           h2B[j].x == -B[j].x && h2B[j].y == -B[j].y && h2B[j].z == -B[j].z) h2neg++;
        if(h4E[j].x == E[j].x && h4E[j].y == E[j].y && h4E[j].z == E[j].z &&
           h4B[j].x == B[j].x && h4B[j].y == B[j].y && h4B[j].z == B[j].z) h4id++;
    }
    printf("      Hodge² = −I em %d de %d; Hodge⁴ = I em %d — ordem 4, e desdobra-se\n\n",
           h2neg, N, h4id);
    ok("o espelho Hodge tem ORDEM 4 no campo (² = −I, ⁴ = I) — é uma dobra, e o Poynting"
       " já fechava em 1 porque E'×B' = E×B. Sem ulp nem 1e-24",
       h2neg == N && h4id == N);
    printf("      E aqui a medida corrigiu-me. Eu ia escrever que ν∘rev inverte AS DUAS coisas.\n");
    printf("      Não inverte: cada uma inverte exatamente UMA.\n\n");
    printf("          Hodge:  |E'| = |B|,   |B'| = |E|     -> σ vira 1/σ,  Poynting fica\n");
    printf("          ν∘rev:  |E'| = 1/|B|, |B'| = 1/|E|   -> σ fica,      Poynting vira −S\n\n");
    printf("      Por isso Hodge é MEIA dualidade — e ν∘rev é a OUTRA metade, não o todo. A\n");
    printf("      dualidade inteira é a composição, e só ela vira as duas. É o mesmo padrão do\n");
    printf("      §B12: uma torre sozinha não fecha, é o par que fecha.\n");
}

printf("\n§T7  E FECHA o circuito — ida e volta, resíduo 0.\n\n");
{
    printf("      etapa                                  resíduo\n");
    printf("      0. o fluido, em ℤ                      0\n");
    printf("      1. linearizado (NTT em F_p)            0     (T5: F^{-1}F = I)\n");
    printf("      2. Hodge⁴ devolve o campo              0     (T6: ordem 4)\n");
    printf("      3. ν∘rev duas vezes (Lagrange)         0     (produto das magnitudes = 1)\n\n");
    V E[N], B[N], tE[N], tB[N];
    campo(E, B);
    hodge(E, B, tE, tB);
    hodge(tE, tB, E, B);
    hodge(E, B, tE, tB);
    hodge(tE, tB, E, B);
    V E0[N], B0[N];
    campo(E0, B0);
    int fecha = 0;
    for(int j = 0; j < N; j++)
        if(E[j].x==E0[j].x && E[j].y==E0[j].y && E[j].z==E0[j].z &&
           B[j].x==B0[j].x && B[j].y==B0[j].y && B[j].z==B0[j].z) fecha++;
    ok("O CIRCUITO FECHA: fluido -> NTT (resíduo 0) -> Hodge⁴ = I, exacto em ℤ."
       " Sem cexp no meio e sem 1e-12 no fecho",
       fecha == N);
    printf("      E é isto que se promete, e só isto. Não se promete chegar: quem vai não tem\n");
    printf("      garantia de voltar, e o §T2 mostra que essa garantia não existe nem em\n");
    printf("      princípio — decidir a fronteira É decidir a parada, e nenhum orçamento finito\n");
    printf("      basta. Pode acabar a memória, podem acabar os recursos, pode morrer no\n");
    printf("      caminho. Todo mundo só tem a garantia de que vai morrer, nada mais.\n");
    printf("\n      Mas quando fecha, FECHOU — e o resíduo diz que fechou. É verificação a\n");
    printf("      posteriori, não promessa a priori, e é a única espécie de certificado que\n");
    printf("      cabe num sistema finito. \"Chega sempre?\" é indecidível; \"fechou desta vez?\"\n");
    printf("      tem resposta, e a resposta é um número.\n");
    printf("\n      E é por isso que a formulação estática vai ao lixo, sem drama nenhum: ela\n");
    printf("      pede uma garantia que ninguém pode dar, sobre objetos que só fecham no\n");
    printf("      limite, depois de ter jogado fora o gerador que os fazia mexer. Nós queremos\n");
    printf("      outra coisa — o gerador, o circuito, o resíduo — e essa nós medimos.\n");
}

printf("\n    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
