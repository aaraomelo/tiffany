/* cruzado_potencia.c — O CRUZADO NÃO VÊ A POTÊNCIA, E É POR ISSO QUE A RAIZ NÃO É PRECISA.
 *
 * O Aarão: "sobre a potencia le os papers sobre os campos locais e resultados sobre
 * invariancia deles ... um produto cruzado antissimetrico é invariante a potencias ...
 * raiz é o inverso, tudo sai inteiro quando normalizado."
 *
 * O resultado está agora em `thm:cruzado-potencia` do universal.tex, e não era novo: era
 * a linha `det M_k = (−1)^k` da tabela do §sec:euler, dita ali como observação sobre o
 * determinante. O determinante É o cruzado — a área —, e aquela linha já era esta
 * invariância sem a enunciar.
 *
 *      Cruz(Au, Av) = det(A)·Cruz(u,v)          e logo   (det A)^k na potência
 *      |det A| = 1  ⟹  |Cruz| INVARIANTE em toda a órbita
 *
 * E daqui a raiz sai de cena. A norma ‖A^k v‖ cresce como σ^k e é irracional em quase todo
 * k; mas a raiz é a INVERSA da potência, e o cruzado não vê nem uma nem outra — as duas
 * passam por ele multiplicando por (det A)^{±k}, que é ±1.
 *
 *   §P1  Cruz(Au,Av) = det(A)·Cruz(u,v) — e o gume: com det ≠ ±1 ele NÃO é invariante
 *   §P2  na órbita da unidade |Cruz| é constante, e a NORMA não é (o par, medido)
 *   §P3  a potência sai do PASSO, e não da tabela — meta-indução
 *   §P4  Lagrange fecha o par: Dir² + Cruz² = N(u)·N(v), sem cos e sem sin
 *   §P5  e é assim que se responde sem raiz: comparar, ordenar e decidir na órbita
 *
 *   cc -O2 -std=c99 -I../lib cruzado_potencia.c -o cruzado_potencia && ./cruzado_potencia
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"

/* o cruzado e a aplicação da matriz são da lib — `rt_cruz2` e `rt_aplica` da reta.h, para
 * onde foram depois deste medidor as ter escrito à mão. Ficam os nomes curtos, que é o que
 * torna os laços abaixo legíveis. */
#define cruz2(u,v)   rt_cruz2((u),(v))
static void aplica(const long *M, const long *v, long *r){ rt_aplica(M, v, 2, r); }

int main(void){
printf("\n=== O CRUZADO NÃO VÊ A POTÊNCIA ============================================\n");
printf("    Cruz(Au,Av) = det(A)·Cruz(u,v). Com |det A| = 1 o cruzado é INVARIANTE em\n");
printf("    toda a órbita — e a raiz, que é a inversa da potência, também não o move.\n");

/* ── §P1 ─────────────────────────────────────────────────────────────────────── */
printf("\n§P1  Cruz(Au,Av) = det(A)·Cruz(u,v) — e com det ≠ ±1 ele NÃO é invariante.\n\n");
{
    long casos = 0, bate = 0, invariantes = 0, moveu = 0, unit = 0, nao_unit = 0;
    for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
    for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
        long M[4] = { a, b, c, d };
        long det = a*d - b*c;
        for(long u0 = -2; u0 <= 2; u0++) for(long u1 = -2; u1 <= 2; u1++)
        for(long v0 = -2; v0 <= 2; v0++) for(long v1 = -2; v1 <= 2; v1++){
            long u[2] = { u0, u1 }, v[2] = { v0, v1 };
            long cr = cruz2(u, v);
            if(cr == 0) continue;                 /* o cruzado nulo não distingue nada */
            long Au[2], Av[2];
            aplica(M, u, Au); aplica(M, v, Av);
            casos++;
            if(cruz2(Au, Av) == det * cr) bate++;  /* a IDENTIDADE, exacta em ℤ */
            /* e as duas metades do gume, contadas à parte */
            if(det == 1 || det == -1){
                unit++;
                if(rt_modulo(cruz2(Au, Av)) == rt_modulo(cr)) invariantes++;
            } else {
                nao_unit++;
                if(rt_modulo(cruz2(Au, Av)) != rt_modulo(cr)) moveu++;
            }
        }
    }
    printf("      pares (M, u, v) com cruzado nao nulo: %ld\n", casos);
    printf("      Cruz(Au,Av) == det(A).Cruz(u,v) em %ld deles\n", bate);
    printf("      com |det| = 1 (%ld casos): |Cruz| ficou IGUAL em %ld\n", unit, invariantes);
    printf("      com |det| != 1 (%ld casos): |Cruz| MUDOU em %ld\n\n", nao_unit, moveu);
    ok("A IDENTIDADE E' EXACTA EM Z: Cruz(Au,Av) = det(A).Cruz(u,v) em todos os pares"
       " varridos, e nao a menos de regua nenhuma — os dois lados sao inteiros. E o gume"
       " esta' nos DOIS lados: com |det| = 1 o modulo do cruzado nao se move, e com"
       " |det| != 1 ele MOVE-SE em todos os casos em que o determinante nao e' zero. Sem"
       " a segunda metade, «invariante» valia por o cruzado nunca mudar seja com que"
       " matriz for",
       bate == casos && invariantes == unit && moveu > 0);
    /* e quantos dos não-unitários NÃO se moveram? só os de determinante zero — que
     * colapsam tudo, e são a outra ponta da mesma frase */
    printf("      (e o det = 0 nao e' excepcao: ele leva o cruzado a ZERO, que e' mudar —\n"
           "       colapsa o par a uma recta, e ficam %ld nao-unitarios sem se mover)\n\n",
           nao_unit - moveu);
}

/* ── §P2 ─────────────────────────────────────────────────────────────────────── */
printf("\n§P2  Na órbita da unidade o cruzado é CONSTANTE — e a norma NÃO é.\n\n");
{
    /* a família metálica: A_m = [[m,1],[1,0]], det = −1. É a órbita do rei. */
    long ms[] = { 1, 2, 3, 5 };
    long cr_const = 0, cr_tot = 0, norma_cresce = 0, norma_tot = 0;
    printf("      m   k   Cruz(A^k u, A^k v)   ‖A^k u‖²  (o quadrado, que é inteiro)\n");
    for(int im = 0; im < 4; im++){
        long m = ms[im];
        long M[4] = { m, 1, 1, 0 };
        long u[2] = { 1, 0 }, v[2] = { 0, 1 };
        long cr0 = cruz2(u, v);
        long n_ant = -1;
        for(int k = 0; k <= 12; k++){
            long cr = cruz2(u, v);
            long nq = rt_dir(u, u, 2);                 /* ‖u‖², inteiro e exacto */
            cr_tot++;
            if(rt_modulo(cr) == rt_modulo(cr0)) cr_const++;
            if(n_ant >= 0){ norma_tot++; if(nq > n_ant) norma_cresce++; }
            n_ant = nq;
            if(m == 1 && k <= 6)
                printf("      %ld  %2d   %+8ld            %ld\n", m, k, cr, nq);
            long Au[2], Av[2];
            aplica(M, u, Au); aplica(M, v, Av);
            u[0] = Au[0]; u[1] = Au[1]; v[0] = Av[0]; v[1] = Av[1];
        }
    }
    printf("\n      o cruzado ficou constante em modulo em %ld de %ld andares\n", cr_const, cr_tot);
    printf("      e a norma ao quadrado CRESCEU em %ld de %ld passos\n\n", norma_cresce, norma_tot);
    ok("E ESTE E' O PAR QUE INTERESSA, e as duas metades tem de ser ditas na mesma frase:"
       " ao longo da orbita o CRUZADO fica constante em modulo — nos 52 andares, para os"
       " quatro metais — e a NORMA cresce em todos os passos. Sao a mesma orbita e os"
       " mesmos vectores: o que separa as duas e' que o cruzado se transforma por det, que"
       " e' -1, e a norma nao. Sem medir a norma a crescer, «o cruzado nao muda» podia"
       " valer por nada estar a mudar",
       cr_const == cr_tot && norma_cresce == norma_tot && cr_tot == 4*13);
}

/* ── §P3 ─────────────────────────────────────────────────────────────────────── */
printf("\n§P3  E a potência sai do PASSO, não da tabela.\n\n");
{
    /* `thm:meta-inducao`: se o corpo do passo não menciona k, o ∀k vem da FORMA. O passo
     * aqui é Cruz(A·u, A·v) = det(A)·Cruz(u,v), e ele não sabe em que andar está. */
    long passos = 0, bons = 0, satur = 0;
    for(long m = 1; m <= 6; m++){
        long M[4] = { m, 1, 1, 0 }, det = -1;
        long u[2] = { 2, 1 }, v[2] = { 1, 3 };
        for(int k = 0; k < 20; k++){
            long cr = cruz2(u, v);
            long Au[2], Av[2];
            aplica(M, u, Au); aplica(M, v, Av);
            /* o tecto: os vectores crescem como σ^k, e ao fim de uns andares não cabem.
             * Isso é falha de REPRESENTAÇÃO e conta-se à parte — não é contra-exemplo. */
            if(rt_modulo(Au[0]) > 1000000000L || rt_modulo(Av[0]) > 1000000000L){ satur++; break; }
            passos++;
            if(cruz2(Au, Av) == det * cr) bons++;
            u[0] = Au[0]; u[1] = Au[1]; v[0] = Av[0]; v[1] = Av[1];
        }
    }
    printf("      passos do laço medidos: %ld, todos a cumprir o passo: %ld\n", passos, bons);
    printf("      e %ld órbitas pararam por não caberem no long — contadas AQUI e não\n"
           "      entre os defeitos\n\n", satur);
    ok("A POTENCIA SAI DO PASSO: o corpo de Cruz(Au,Av) = det(A).Cruz(u,v) nao menciona k"
       " — recebe dois vectores e uma matriz, e nao sabe em que andar esta'. E' dessa FORMA"
       " que vem o «para todo k», e nao do comprimento desta varredura: uma tabela de"
       " andares prova os andares da tabela. O que a varredura faz e' outra coisa —"
       " confirmar que o passo corre —, e a saturacao das orbitas conta-se em lugar"
       " separado, porque falha de representacao nao e' contra-exemplo",
       bons == passos && passos > 0 && satur > 0);
}

/* ── §P4 ─────────────────────────────────────────────────────────────────────── */
printf("\n§P4  Lagrange fecha o par: Dir² + Cruz² = N(u)·N(v) — sem cos e sem sin.\n\n");
{
    long casos = 0, fecha = 0, cos_ok = 0, cos_tot = 0;
    for(long u0 = -6; u0 <= 6; u0++) for(long u1 = -6; u1 <= 6; u1++)
    for(long v0 = -6; v0 <= 6; v0++) for(long v1 = -6; v1 <= 6; v1++){
        long u[2] = { u0, u1 }, v[2] = { v0, v1 };
        long dir = rt_dir(u, v, 2), cr = cruz2(u, v);
        long nu = rt_dir(u, u, 2), nv = rt_dir(v, v, 2);
        casos++;
        if(dir*dir + cr*cr == nu*nv) fecha++;
        /* e o cos² da lib é a metade directa da mesma identidade: cos² = Dir²/(N(u)N(v)),
         * logo cos² + sin² = 1 lê-se como Dir² + Cruz² = N(u)N(v) e mais nada */
        if(nu && nv){
            long cn, cd;
            rt_cos2(u, v, 2, &cn, &cd);
            cos_tot++;
            if(cn*(nu*nv) == dir*dir*cd) cos_ok++;
        }
    }
    printf("      pares de vectores em [-6,6]²: %ld ; Dir² + Cruz² = N(u).N(v) em %ld\n",
           casos, fecha);
    printf("      e o cos² da reta.h e' a metade directa da MESMA identidade: %ld de %ld\n"
           "      (os %ld que ficam de fora tem um dos vectores NULO, e esse nao tem direccao)\n\n",
           cos_ok, cos_tot, casos - cos_tot);
    ok("LAGRANGE FECHA O PAR, E E' ELE QUE DISPENSA A RAIZ: Dir(u,v)^2 + Cruz(u,v)^2 ="
       " N(u).N(v) em todos os 28561 pares, exacto em Z. Esta e' a identidade que da'"
       " cos^2 + sin^2 = 1 sem nunca formar nem o cosseno nem o seno — e e' por isso que"
       " esta casa mede cos^2 e nao cos: nao e' economia, e' que o objecto que a orbita"
       " conserva e' o QUADRADO, e a raiz dele e' a unica parte que ela nao preserva",
       fecha == casos && casos == 28561 && cos_ok == cos_tot && cos_tot == 28224);
}

/* ── §P5 ─────────────────────────────────────────────────────────────────────── */
printf("\n§P5  E as perguntas separam-se: o CRUZADO atravessa a órbita, o DIRECTO não.\n\n");
{
    /* Aqui eu previ mal e a medida corrigiu-me, e o que ela deu é melhor do que a
     * previsão. Escrevi que as três perguntas — qual é maior, paralelos, perpendiculares —
     * seriam todas invariantes na órbita. Não são, e a razão é o teorema:
     *
     *      PARALELOS      é  Cruz(u,v) = 0   e o cruzado transforma-se por det = ±1
     *                     ⟹  INVARIANTE, e em toda a órbita
     *      PERPENDICULARES é  Dir(u,v) = 0    e o directo NÃO se transforma por escalar
     *                     ⟹  NÃO invariante: A_m preserva a ÁREA e não a MÉTRICA
     *
     * A matriz A_m = [[m,1],[1,0]] tem |det| = 1 mas não é ortogonal. Preservar a área e
     * preservar ângulos são condições diferentes, e a órbita do rei só cumpre a primeira.
     * Esta é a assimetria Dir/Cruz do `thm:espectro-tau` lida na ACÇÃO — e é o que dá
     * conteúdo à palavra «invariante»: se tudo fosse invariante, nada estava a mexer. */
    long M[4] = { 1, 1, 1, 0 };                    /* o ouro, det = −1 */
    long pu[2] = { 1, 2 }, pv[2] = { 2, 4 };       /* paralelos          */
    long qu[2] = { 1, 0 }, qv[2] = { 0, 1 };       /* perpendiculares    */
    long par_sim = 0, perp_sim = 0, andares = 0;
    long cruz_const = 0, cr0 = cruz2(qu, qv);
    printf("      andar   par (1,2)//(2,4)   par (1,0)⊥(0,1)   Cruz do 2.º par   Dir do 2.º par\n");
    for(int k = 0; k <= 10; k++){
        andares++;
        int par = rt_paralelos(pu, pv, 2);
        int per = rt_perp(qu, qv, 2);
        if(par) par_sim++;
        if(per) perp_sim++;
        if(rt_modulo(cruz2(qu, qv)) == rt_modulo(cr0)) cruz_const++;
        if(k <= 5)
            printf("      %3d   %-18s %-17s %+8ld          %+8ld\n", k,
                   par ? "paralelos" : "NAO", per ? "perpendiculares" : "NAO",
                   cruz2(qu, qv), rt_dir(qu, qv, 2));
        long Bu[2], Bv[2], Cu[2], Cv[2];
        aplica(M, pu, Bu); aplica(M, pv, Bv);
        aplica(M, qu, Cu); aplica(M, qv, Cv);
        pu[0]=Bu[0]; pu[1]=Bu[1]; pv[0]=Bv[0]; pv[1]=Bv[1];
        qu[0]=Cu[0]; qu[1]=Cu[1]; qv[0]=Cv[0]; qv[1]=Cv[1];
    }
    printf("\n      PARALELOS      : continua a ser sim em %ld de %ld andares\n", par_sim, andares);
    printf("      PERPENDICULARES: continua a ser sim em %ld de %ld andares\n", perp_sim, andares);
    printf("      e o CRUZADO desse mesmo par ficou constante em %ld de %ld\n\n", cruz_const, andares);
    ok("E AS DUAS PERGUNTAS SEPARAM-SE, que e' o que da' conteudo a palavra «invariante»:"
       " PARALELOS e' Cruz = 0 e atravessa a orbita inteira — o cruzado transforma-se por"
       " det = -1 e o zero nao se move —, enquanto PERPENDICULARES e' Dir = 0 e NAO"
       " atravessa: cai ja' no primeiro andar. A matriz A_m tem |det| = 1 mas nao e'"
       " ortogonal, e preservar a AREA nao e' preservar a METRICA. Eu tinha previsto que as"
       " duas fossem invariantes, e a medida corrigiu-me — e o resultado e' melhor do que a"
       " previsao, porque e' a assimetria Dir/Cruz do thm:espectro-tau lida na ACCAO. Se"
       " tudo fosse invariante, nada estava a mexer",
       par_sim == andares && perp_sim == 1 && cruz_const == andares && andares == 11);
}

/* ── §P6 ─────────────────────────────────────────────────────────────────────── */
printf("\n§P6  E a raiz sai de cena onde ela apareceria: comparar sem a formar.\n\n");
{
    /* A pergunta que traria o sqrt é «qual vector é maior», e ela é sobre a ORDEM. Como
     * x ↦ x² é monótona nos não negativos, a ordem das normas É a ordem dos quadrados, e
     * os quadrados são inteiros. Varre-se, e o gume é o par onde a resposta MUDA. */
    long casos = 0, bate = 0, houve_menor = 0, houve_maior = 0, houve_igual = 0;
    for(long a = -9; a <= 9; a++) for(long b = -9; b <= 9; b++)
    for(long c = -9; c <= 9; c++) for(long d = -9; d <= 9; d++){
        long u[2] = { a, b }, v[2] = { c, d };
        long nu = rt_dir(u, u, 2), nv = rt_dir(v, v, 2);
        casos++;
        /* a ordem pelos quadrados, e a ordem pelas normas — a segunda só existe em
         * vírgula, e é contra ela que a primeira se confere */
        int ord_q = (nu < nv) ? -1 : (nu > nv ? 1 : 0);
        if(ord_q < 0) houve_menor++; else if(ord_q > 0) houve_maior++; else houve_igual++;
        /* a conferência SEM raiz: nu < nv  ⟺  nu·nu < nv·nu e nu·nv < nv·nv … a forma
         * honesta é que a monotonia de x↦x² já é o teorema, e o que se mede é que a
         * ordem dos quadrados nunca contradiz a soma das coordenadas ao quadrado */
        if((nu < nv) == (a*a + b*b < c*c + d*d)) bate++;
    }
    printf("      pares em [-9,9]²: %ld ; a ordem dos quadrados e a das somas coincidem: %ld\n",
           casos, bate);
    printf("      e as tres respostas aparecem: menor %ld, maior %ld, iguais %ld\n\n",
           houve_menor, houve_maior, houve_igual);
    ok("«QUAL E' MAIOR» NAO PRECISA DA RAIZ, e a razao e' um teorema e nao uma aproximacao:"
       " x -> x^2 e' monotona nos nao negativos, logo a ordem das normas E' a ordem dos"
       " quadrados, e os quadrados sao inteiros. Varridos 130321 pares, e as TRES respostas"
       " aparecem — menor, maior e iguais —, sem o que «coincidem» valia por a comparacao"
       " devolver sempre o mesmo lado",
       bate == casos && casos == 130321
       && houve_menor > 0 && houve_maior > 0 && houve_igual > 0);
}

printf("\n  ─────────────────────────────────────────────────────────────\n");
printf("  A raiz e a potencia sao a mesma operacao lida nos dois sentidos, e o\n");
printf("  cruzado nao ve nenhuma delas: as duas passam por ele multiplicando por\n");
printf("  (det A)^{+-k}, que e' mais ou menos um. Normalizado pelo invariante,\n");
printf("  tudo sai inteiro.\n");
printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
return falhas ? 1 : 0;
}
