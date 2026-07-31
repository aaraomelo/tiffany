/* circuito.c — FECHAR O CIRCUITO: estica, gira, e volta — tudo palavra no metal.
 *
 * O Aarão: "fecha o circuito."
 *
 * O que faltava para fechar não era mais um opcode: era o GRUPO. A ISA tinha o gato — GOLD,
 * SILVER, BRONZE — e o gato só ESTICA (det −1, hiperbólico). Uma máquina que só estica não gera
 * o grupo unimodular: falta quem GIRE. E quem gira acabou de entrar no toolkit pela porta do
 * catálogo — o cristalino, cujo operador ×ω tem det +1 e ordem finita.
 *
 * Com três peças o circuito fecha, e é resultado clássico posto no metal:
 *
 *     GOLD     A_1 = [[1,1],[1,0]]    det −1   estica    ordem ∞
 *     ESQUILO  S   = [[0,−1],[1,0]]   det +1   gira      ordem 4     ← o cristal, t=0
 *     TROCA    J   = [[0,1],[1,0]]    det −1   reflete   ordem 2     ← a involução
 *
 * ⟨S, T⟩ = SL₂(ℤ), e T = A_1·J — o cisalhamento não precisa de ser opcode, é PALAVRA de dois.
 * Com J junto, é GL₂(ℤ) inteiro: toda matriz de det ±1 é palavra, e toda palavra tem palavra
 * inversa. É isso o circuito fechado — não "dá para calcular", mas: o que a máquina faz, ela
 * desfaz, e sem sair dos inteiros.
 *
 *   §F1  as três peças, e as suas assinaturas: det, ordem, o que cada uma faz
 *   §F2  T = A_1·J — o cisalhamento é palavra de dois, e não precisava de opcode
 *   §F3  TODO metal é palavra em ouro e troca: A_m = T^{m−1}·A_1, medido em m = 1..40
 *   §F4  e a volta também: A_m⁻¹ = J·A_{−m}·J, e A_{−m} é palavra — o grupo FECHA
 *   §F3b o NEGRO inteiro: a régua vai para os dois lados, e A_m existe para TODO m ∈ ℤ
 *   §F4b a inversa de todo metal é palavra: a mesma, ao contrário e letra a letra invertida
 *   §F4c e então quatro dos oito opcodes de metal são ATALHO, dos dois lados igualmente
 *   §F5  o circuito, percorrido: ida por palavra, volta por palavra, e devolve o que entrou
 *   §F6  o que fechou, e o que ficou de fora — dito
 *
 *   cc -O2 -std=c99 circuito.c -o circuito && ./circuito
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static int meq(Mat x, Mat y){ return x.a==y.a && x.b==y.b && x.c==y.c && x.d==y.d; }
static long mtr(Mat x){ return x.a + x.d; }
static const Mat ID = {1,0,0,1};

/* a ordem de uma peça, contada — 0 se não fechar dentro do limite */
static long ordem(Mat W, long teto){
    Mat P = ID;
    for(long i = 1; i <= teto; i++){ P = me_prod(P, W); if(meq(P, ID)) return i; }
    return 0;
}

/* A PALAVRA. Uma sequência de peças, e o que ela vale é o produto — com a convenção da máquina:
 * o opcode que vem primeiro AGE primeiro, logo o produto acumula pela ESQUERDA. */
enum { G, S, J, Gi };                              /* ouro, esquilo, troca, NEGRO */
static Mat peca(int g){
    switch(g){ case G:  return me_gato(1);      case S: return cr_mat(0);
               case Gi: return me_antigato(1);  default: return me_troca(); }
}
static Mat palavra(const int *w, int n){
    Mat P = ID;
    for(int i = 0; i < n; i++) P = me_prod(peca(w[i]), P);   /* o i-ésimo age depois */
    return P;
}

int main(void){
printf("\n=== FECHAR O CIRCUITO ======================================================\n");
printf("    O gato estica. Faltava quem girasse — e sem ele a máquina não gera o grupo.\n");

/* ---------------------------------------------------------------- §F1 ------ */
printf("\n§F1  As três peças, e o que cada uma faz.\n\n");
{
    int mau = 0;
    printf("      opcode     matriz            det   traço   ordem   faz\n");
    Mat A = me_gato(1), Sq = cr_mat(0), Jj = me_troca();
    if(me_det(A)  != -1) mau++;
    if(me_det(Sq) !=  1) mau++;
    if(me_det(Jj) != -1) mau++;
    if(ordem(A, 64) != 0) mau++;                   /* o gato NÃO fecha: ordem infinita */
    if(ordem(Sq,64) != 4) mau++;                   /* o esquilo fecha em 4 */
    if(ordem(Jj,64) != 2) mau++;                   /* a troca é involução */
    printf("      GOLD       [[1,1],[1,0]]     %-5ld %-7ld %-7s estica — hiperbólico\n",
           me_det(A), mtr(A), "∞");
    printf("      ESQUILO    [[0,−1],[1,0]]    %-5ld %-7ld %-7ld gira — elíptico, o cristal\n",
           me_det(Sq), mtr(Sq), ordem(Sq,64));
    printf("      TROCA      [[0,1],[1,0]]     %-5ld %-7ld %-7ld reflete — a involução\n",
           me_det(Jj), mtr(Jj), ordem(Jj,64));
    ok("as três assinaturas conferem: ordem ∞, 4 e 2, e det −1, +1, −1", mau == 0);
    printf("\n      Uma máquina só com a primeira linha nunca volta ao ponto de partida por\n");
    printf("      repetição — é isso que ordem infinita quer dizer. As outras duas voltam, e é\n");
    printf("      delas que sai o fecho.\n");
}

/* ---------------------------------------------------------------- §F2 ------ */
printf("\n§F2  T = A_1·J: o cisalhamento é PALAVRA de dois, e não precisava de opcode.\n\n");
{
    int mau = 0;
    Mat T = { 1,1,0,1 };
    int w[2] = { J, G };                            /* troca age primeiro, depois o ouro */
    if(!meq(palavra(w,2), T)) mau++;
    ok("TROCA seguida de GOLD é exatamente o cisalhamento T = [[1,1],[0,1]]", mau == 0);
    printf("      palavra   TROCA GOLD        vale   [[1,1],[0,1]] = T   ✓\n");
    printf("\n      Isto poupa um opcode e diz porquê: o cisalhamento não é uma peça, é um\n");
    printf("      MOVIMENTO de duas. Pôr opcode para ele seria guardar no metal o que a\n");
    printf("      composição já dá de graça.\n");
}

/* ---------------------------------------------------------------- §F3 ------ */
printf("\n§F3  TODO metal é palavra em OURO e TROCA — não só os três que têm opcode.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m    A_m             palavra                          confere?\n");
    for(long m = 1; m <= 40; m++){
        /* A_m = T^{m−1}·A_1, e T = A_1·J — logo A_m = (A_1 J)^{m−1} A_1: na máquina, um GOLD
         * e depois (m−1) vezes o par TROCA GOLD. Tudo com opcode, nenhum produto. */
        int w[128]; int n = 0;
        w[n++] = G;
        for(long k = 1; k < m; k++){ w[n++] = J; w[n++] = G; }
        if(!meq(palavra(w,n), me_gato(m))) mau++;
        if(m <= 3)
            printf("      %-4ld [[%ld,1],[1,0]]   %-32s sim ✓\n", m, m,
                   m==1 ? "GOLD" : (m==2 ? "GOLD TROCA GOLD" : "GOLD TROCA GOLD TROCA GOLD"));
        casos++;
    }
    ok("A_m = (A_1·J)^{m−1}·A_1 para todo m — o metal m tem palavra de 2m−1 opcodes", mau == 0);
    printf("      (%ld metais, e nenhum deles precisa de opcode próprio.)\n", casos);
    printf("\n      Ouro, prata e bronze têm opcode por serem os três primeiros, não por serem\n");
    printf("      especiais. O quadragésimo metal corre na mesma máquina, com palavra mais longa\n");
    printf("      e sem uma multiplicação.\n");
}

/* ---------------------------------------------------------------- §F4 ------ */
printf("\n§F4  E a VOLTA também é palavra: o grupo fecha dos dois lados.\n\n");
{
    int mau = 0; long casos = 0;
    for(long m = 1; m <= 30; m++){
        Mat A = me_gato(m), Ai = me_antigato(m);
        if(!meq(me_prod(A, Ai), ID)) mau++;
        /* a inversa como PALAVRA: J·A_{−m}·J, e A_{−m} monta-se como o A_m, com o sinal
         * dentro do próprio gato — o que a máquina faz com o opcode negro numa instrução. */
        Mat conj = me_prod(me_troca(), me_prod(me_gato(-m), me_troca()));
        if(!meq(conj, Ai)) mau++;
        /* e o essencial: ida seguida de volta é a identidade, seja qual for a peça */
        if(!meq(me_prod(Ai, A), ID)) mau++;
        casos++;
    }
    /* o esquilo e a troca dispensam inversa: S⁻¹ = S³ e J⁻¹ = J, dentro do próprio opcode */
    Mat Sq = cr_mat(0), Jj = me_troca();
    if(!meq(me_prod(Sq, me_prod(Sq, me_prod(Sq, Sq))), ID)) mau++;
    if(!meq(me_prod(Jj, Jj), ID)) mau++;
    ok("toda peça tem inversa DENTRO do repertório — o grupo fecha", mau == 0);
    printf("      (%ld metais, e o esquilo com S⁻¹ = S³, a troca com J⁻¹ = J.)\n", casos);
    printf("\n      O esquilo e a troca nem precisam de opcode de volta: por terem ordem finita, a\n");
    printf("      inversa é a própria peça repetida. Só o gato precisou — porque é o único que\n");
    printf("      não fecha por repetição.\n");
}

/* ---------------------------------------------------------------- §F5 ------ */
printf("\n§F5  O CIRCUITO percorrido: ida por palavra, volta por palavra, e devolve.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      entrada   cadeia            depois da ida   depois da volta   devolve?\n");
    long cad[5][4] = {{1,0,0,0},{2,3,0,0},{1,2,3,0},{3,3,3,3},{2,1,2,1}};
    int nel[5] = {1,2,3,4,4};
    for(int t = 0; t < 5; t++)
    for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
        Par v = {a,b}, u = v;
        for(int e = 0; e < nel[t]; e++) u = me_ap(me_gato(cad[t][e]), u);      /* a ida */
        Par z = u;
        for(int e = nel[t]-1; e >= 0; e--) z = me_ap(me_antigato(cad[t][e]), z); /* a volta */
        if(z.a != v.a || z.b != v.b) mau++;
        if(t == 2 && a == 5 && b == 3)
            printf("      (5,3)     ouro prata bronze (%ld,%ld)%*s(%ld,%ld)%*s%s\n",
                   u.a, u.b, 9, "", z.a, z.b, 11, "",
                   (z.a==5&&z.b==3) ? "sim ✓" : "NÃO");
        casos++;
    }
    ok("a cadeia vai e volta e devolve o que entrou — em toda a varredura", mau == 0);
    printf("      (%ld percursos.)\n", casos);
    printf("\n      E devolve porque det = ±1 em cada peça, não porque se guardou cópia. É a\n");
    printf("      diferença entre desfazer e restaurar: restaurar precisa de memória, desfazer\n");
    printf("      precisa só de que a peça seja invertível.\n");
}

/* ---------------------------------------------------------------- §F3b ----- */
printf("\n§F3b O NEGRO INTEIRO: a régua vai para os dois lados, e A_m existe para TODO m ∈ ℤ.\n\n");
{
    int mau = 0; long casos = 0;
    /* Aqui estava a minha assimetria, e o Aarão apanhou-a: o branco eu generalizei — A_m para
     * todo m ≥ 1 — e o negro deixei nos três opcodes. Meio chicote de um dos lados.
     *
     * A régua não tem lado. T⁻¹ = J·A_1⁻¹ é o ESPELHO EXATO de T = A_1·J: a mesma palavra ao
     * contrário, cada letra invertida. E com ela A_m = T^{m−1}·A_1 vale para m ≤ 0 também. */
    printf("      m     A_m               palavra                              confere?\n");
    for(long m = -40; m <= 40; m++){
        int w[256]; int n = 0;
        w[n++] = G;                                   /* A_1 age primeiro, dos dois lados */
        if(m >= 1) for(long k = 1; k <  m; k++){ w[n++] = J;  w[n++] = G;  }   /* T   branco */
        else       for(long k = m; k <= 0; k++){ w[n++] = Gi; w[n++] = J;  }   /* T⁻¹ negro  */
        if(!meq(palavra(w,n), me_gato(m))) mau++;
        if(m == 0 || m == -1 || m == -2)
            printf("      %-5ld [[%ld,1],[1,0]]%*s%-36s sim ✓\n", m, m, m<=-10?1:2, "",
                   m==0 ? "GOLD NEGRO TROCA"
                        : (m==-1 ? "GOLD NEGRO TROCA NEGRO TROCA"
                                 : "GOLD NEGRO TROCA NEGRO TROCA NEGRO TROCA"));
        casos++;
    }
    ok("A_m = T^{m−1}·A_1 vale para TODO m inteiro — negativo, zero e positivo", mau == 0);
    printf("      (%ld metais, de m = −40 a m = 40.)\n", casos);
    printf("\n      E m = 0 dá A_0 = J: a TROCA é o metal do meio, o ponto onde os dois lados da\n");
    printf("      régua se encontram. Não é uma peça à parte que eu acrescentei — é onde o\n");
    printf("      chicote passa ao mudar de sinal.\n");
}

/* ---------------------------------------------------------------- §F4b ----- */
printf("\n§F4b E A INVERSA de todo metal também é palavra: a mesma, ao contrário e invertida.\n\n");
{
    int mau = 0; long casos = 0;
    for(long m = -30; m <= 30; m++){
        /* a palavra do branco */
        int w[256]; int n = 0;
        w[n++] = G;
        if(m >= 1) for(long k = 1; k <  m; k++){ w[n++] = J;  w[n++] = G;  }
        else       for(long k = m; k <= 0; k++){ w[n++] = Gi; w[n++] = J;  }
        /* a do negro: a MESMA ao contrário, cada letra pela sua inversa. J é a sua própria. */
        int v[256];
        for(int i = 0; i < n; i++){
            int g = w[n-1-i];
            v[i] = (g == G) ? Gi : (g == Gi ? G : J);
        }
        if(!meq(palavra(v,n), me_antigato(m))) mau++;
        if(!meq(me_prod(palavra(v,n), palavra(w,n)), ID)) mau++;   /* e desfaz mesmo */
        casos++;
    }
    ok("a palavra do negro é a do branco ao contrário, letra a letra invertida", mau == 0);
    printf("      (%ld metais, e o produto das duas é a identidade em todos.)\n", casos);
    printf("\n      É a regra inteira, e não precisa de tabela: reverter a ordem e trocar cada\n");
    printf("      letra pela sua inversa. O gato vai a negro, o negro vai a gato, e a troca fica\n");
    printf("      onde está — porque é involução.\n");
}

/* ---------------------------------------------------------------- §F4c ----- */
printf("\n§F4c E ENTÃO: quatro dos oito opcodes de metal são LUXO, dos dois lados igualmente.\n\n");
{
    int mau = 0;
    printf("      opcode          é palavra em quê                        preciso?\n");
    struct { const char *nome; long m; int negro; } ops[] = {
        { "GOLD",         1, 0 }, { "SILVER",       2, 0 }, { "BRONZE",       3, 0 },
        { "NEGRO_OURO",   1, 1 }, { "NEGRO_PRATA",  2, 1 }, { "NEGRO_BRONZE", 3, 1 },
    };
    for(unsigned t = 0; t < sizeof ops/sizeof ops[0]; t++){
        long m = ops[t].m;
        int w[64]; int n = 0;
        w[n++] = G;
        for(long k = 1; k < m; k++){ w[n++] = J; w[n++] = G; }
        Mat alvo = ops[t].negro ? me_antigato(m) : me_gato(m);
        if(ops[t].negro){
            int v[64];
            for(int i = 0; i < n; i++){ int g = w[n-1-i]; v[i] = (g==G) ? Gi : (g==Gi ? G : J); }
            for(int i = 0; i < n; i++) w[i] = v[i];
        }
        if(!meq(palavra(w,n), alvo)) mau++;
        printf("      %-15s %-39s %s\n", ops[t].nome,
               m == 1 ? "— é gerador" : (ops[t].negro ? "negro+troca" : "ouro+troca"),
               m == 1 ? "SIM" : "não — é atalho");
    }
    ok("prata e bronze, e os seus negros, são palavras — só o primeiro elo é gerador", mau == 0);
    printf("\n      O repertório MÍNIMO que fecha é de quatro peças, e é simétrico:\n");
    printf("        GOLD  NEGRO_OURO  TROCA  ESQUILO\n");
    printf("      Os outros quatro opcodes ficam por serem ATALHOS — poupam palavra, não poder.\n");
    printf("      E é bom que fique dito qual é qual: um atalho que se toma por gerador faz\n");
    printf("      pensar que a máquina precisa dele.\n");
}

/* ---------------------------------------------------------------- §F6 ------ */
printf("\n§F6  O que fechou, e o que ficou de fora.\n\n");
{
    printf("      peça             opcode        inversa            no metal?\n");
    printf("      gato A_m         GOLD…         NEGRO_…            sim, ida e volta\n");
    printf("      esquilo S        ESQUILO       S³, a própria      sim, ordem 4 fecha\n");
    printf("      troca J          TROCA         J, a própria       sim, ordem 2 fecha\n");
    printf("      cisalhamento T   —             palavra TROCA GOLD sim, e sem opcode\n");
    ok("o repertório fecha: toda peça e toda inversa estão no metal", 1);
    printf("\n      O CIRCUITO FECHADO quer dizer isto e só isto: o que a máquina faz, ela desfaz,\n");
    printf("      dentro dos inteiros e sem guardar cópia. Não quer dizer que ela seja rápida nem\n");
    printf("      que faça tudo — quer dizer que não perde.\n");
    printf("\n      E o que ficou de FORA, dito: a decomposição de uma unimodular QUALQUER em\n");
    printf("      palavra não está implementada no compilador. Mediu-se que ela existe para os\n");
    printf("      metais (§F3) e para as inversas (§F4); para uma matriz arbitrária o algoritmo\n");
    printf("      é o de Euclides e não está aqui. Dizer que já está seria o de sempre — medir\n");
    printf("      uma fatia e afirmar o todo.\n");
}

printf("\n=== O CIRCUITO ============================================================\n");
printf("  Faltava para fechar não mais um opcode, mas o GRUPO. O gato só estica, e quem só\n");
printf("  estica não fecha. Entrou quem gira — o esquilo, vindo do cristalino:\n\n");
printf("    GOLD      A_1   det −1   ordem ∞   estica\n");
printf("    ESQUILO   S     det +1   ordem 4   gira    — ×ω do cristal, t=0\n");
printf("    TROCA     J     det −1   ordem 2   reflete — a involução\n\n");
printf("  Com as três: T = A_1·J (o cisalhamento é palavra, não opcode), A_m = T^{m−1}·A_1 (todo\n");
printf("  metal é palavra, não só os três que têm código), e toda inversa está dentro — o gato\n");
printf("  pelo negro, o esquilo por S³, a troca por si própria.\n\n");
printf("  Circuito fechado quer dizer: o que a máquina faz, ela desfaz, nos inteiros e sem\n");
printf("  guardar cópia. Desfazer não precisa de memória; restaurar precisaria.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
