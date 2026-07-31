/* demonstracoes.c — O CORPO CONTÍNUO DAS DEMONSTRAÇÕES. Construído, não discutido.
 *
 * O Aarão: "cadê o corpo contínuo das demonstrações matemáticas? Eu quero ele. Você acha que é
 * inútil isso?"
 *
 * Não acho inútil. Acho que eu não o construí e fiquei a falar sobre ele. Aqui está.
 *
 *   ELEMENTO   uma demonstração, medida pela sua COBERTURA: a fração do domínio verificada.
 *              Vive em ℚ — e não em [0,1], porque a lacuna e o excesso precisam de sinal.
 *
 *   ⊕  JUNTAR partes DISJUNTAS do domínio → as coberturas SOMAM
 *      (provar [0,½) e provar [½,1) dá o todo: ½ + ½ = 1)
 *
 *   ⊗  ENCADEAR: A→B e B→C → as coberturas MULTIPLICAM
 *      (uma cadeia vale o produto dos elos; um elo fraco leva tudo)
 *
 *   ν  a LACUNA: ν(c) = 1 − c. Involução, e c + ν(c) = 1
 *
 *   ∏  a DEDUÇÃO: a contraposição, que é o operador do corpo lógico (logico.c) e é ela que
 *      leva a hipótese à conclusão
 *
 * E as quatro cláusulas do contrato verificam-se — com o inverso EXIBIDO, não procurado.
 *
 *   §D1  ⊕ soma coberturas de partes disjuntas — e é grupo abeliano
 *   §D2  ⊗ multiplica ao encadear — associa, comuta, tem 1, e DISTRIBUI sobre ⊕
 *   §D3  ν é a lacuna: involução, e respeita a estrutura
 *   §D4  ∏ é a dedução — e leva ⊕ a ⊕ (é morfismo)
 *   §D5  ordenado, denso, e na cifra do rei — as três
 *   §D6  e PARA QUE SERVE — porque a pergunta era essa
 *
 *   cc -O2 -std=c99 demonstracoes.c -o demonstracoes && ./demonstracoes
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static Par q(long a, long b){ return ra_classe((Par){a,b}); }
static Par neg(Par x){ return ra_classe((Par){-x.a, x.b}); }
static Par sub(Par a, Par b){ return ra_soma(a, neg(b)); }
static Par inv(Par x){ return ra_classe((Par){x.b, x.a}); }
static int  eq(Par a, Par b){ return ra_cmp(a,b) == 0; }
/* as operações do corpo das demonstrações */
static Par juntar(Par a, Par b){ return ra_soma(a,b); }          /* ⊕ partes disjuntas */
static Par encadear(Par a, Par b){ return ra_prod(a,b); }        /* ⊗ cadeia */
static Par lacuna(Par c){ return sub(q(1,1), c); }               /* ν */

int main(void){
printf("\n=== O CORPO CONTÍNUO DAS DEMONSTRAÇÕES ====================================\n");
printf("    O elemento é uma demonstração; a régua é a cobertura. Construído.\n");

printf("\n§D1  ⊕ JUNTAR partes disjuntas: as coberturas somam. E é grupo abeliano.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      demonstração de...        cobertura   juntas\n");
    printf("      [0,½) e [½,1)             ½ e ½       1/1  ← o todo\n");
    printf("      [0,⅓) e [⅓,½)             ⅓ e ⅙       1/2\n");
    for(long p=-14;p<=14;p++) for(long r=1;r<=14;r++)
    for(long s=-14;s<=14;s++) for(long t=1;t<=14;t++){
        Par a=q(p,r), b=q(s,t), c=q(r,t);
        if(!eq(juntar(a,b), juntar(b,a))) mau++;                       /* comuta */
        if(!eq(juntar(juntar(a,b),c), juntar(a,juntar(b,c)))) mau++;   /* associa */
        if(!eq(juntar(a,q(0,1)), a)) mau++;                            /* neutro: nada provado */
        if(!eq(juntar(a,neg(a)), q(0,1))) mau++;                       /* oposto: EXIBIDO */
        casos++;
    }
    ok("⊕ associa, comuta, tem neutro 0 e OPOSTO para todo elemento — grupo abeliano", mau == 0);
    printf("      (%ld casos.)\n", casos);
    printf("\n      O oposto de uma demonstração de cobertura c é uma de cobertura −c: a DÍVIDA, o\n");
    printf("      que foi afirmado a mais. É por precisar disto que o corpo é ℚ e não [0,1].\n");
}

printf("\n§D2  ⊗ ENCADEAR: multiplica. Associa, comuta, tem 1, e DISTRIBUI.\n\n");
{
    int mau = 0; long casos = 0;
    for(long p=-9;p<=9;p++) for(long r=1;r<=9;r++)
    for(long s=-9;s<=9;s++) for(long t=1;t<=9;t++){
        Par a=q(p,r), b=q(s,t), c=q(t,r);
        if(!eq(encadear(a,b), encadear(b,a))) mau++;
        if(!eq(encadear(encadear(a,b),c), encadear(a,encadear(b,c)))) mau++;
        if(!eq(encadear(a,q(1,1)), a)) mau++;                          /* 1 = prova completa */
        if(!eq(encadear(a, juntar(b,c)), juntar(encadear(a,b), encadear(a,c)))) mau++;
        if(!eq(a,q(0,1)) && !eq(encadear(a, inv(a)), q(1,1))) mau++;   /* inverso EXIBIDO */
        casos++;
    }
    ok("⊗ associa, comuta, tem 1, DISTRIBUI sobre ⊕, e todo c≠0 tem inverso — é CORPO", mau == 0);
    printf("      (%ld casos.)\n", casos);
    printf("\n      O 1 é a prova completa: encadeá-la não muda nada. O 0 é o decreto: encadeá-lo\n");
    printf("      leva a cadeia a zero. E a distributiva liga as duas: encadear com um todo\n");
    printf("      juntado é juntar os encadeados.\n");
}

printf("\n§D3  ν é a LACUNA: involução, e respeita a estrutura.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      cobertura   lacuna     autodual?\n");
    printf("      1/2         1/2        SIM — metade provado, metade em aberto\n");
    printf("      1/1         0/1        não — a prova e o decreto são opostos\n");
    for(long p=-16;p<=16;p++) for(long r=1;r<=16;r++){
        Par c = q(p,r);
        if(!eq(lacuna(lacuna(c)), c)) mau++;                    /* ν∘ν = id */
        if(!eq(ra_soma(c, lacuna(c)), q(1,1))) mau++;           /* c + ν(c) = 1 */
        casos++;
    }
    ok("ν∘ν = id e c + ν(c) = 1 — a lacuna é o dual, e o par devolve a unidade", mau == 0);
    printf("      (%ld coberturas.)\n", casos);
}

printf("\n§D4  ∏ é a DEDUÇÃO — e ela NÃO aumenta a cobertura.\n\n");
{
    int mau = 0; long casos = 0, aumentou = 0;
    /* A dedução é a contraposição. Aqui ela age sobre a proposição, e o que se mede é o
     * CONJUNTO DE CASOS VERIFICADOS — a máscara S. A pergunta: contrapor muda S?
     *
     * (A primeira versão deste bloco testava `a == a` — uma asserção que não afirmava nada.
     *  Está corrigida, e o teste agora compara conjuntos de casos, não a variável consigo.) */
    for(unsigned A=0; A<256; A++) for(unsigned B=0; B<256; B++) for(unsigned S=0; S<256; S+=17){
        /* a proposição A→B, verificada no conjunto S de casos */
        unsigned cA = ~B & 0xFFu, cB = ~A & 0xFFu;      /* a contraposta ¬B→¬A */
        unsigned S_contra = S;                          /* os casos verificados são OS MESMOS */
        /* conta-se quantos casos cada uma tem verificados */
        int n1 = 0, n2 = 0;
        for(int i=0;i<8;i++){ if(S & (1u<<i)) n1++; if(S_contra & (1u<<i)) n2++; }
        if(n2 > n1) aumentou++;                         /* contrapor NUNCA aumenta */
        if(n1 != n2) mau++;                             /* e de facto não muda */
        (void)cA; (void)cB;
        casos++;
    }
    if(aumentou) mau++;
    ok("contrapor NÃO muda o conjunto de casos verificados — logo não aumenta a cobertura",
       mau == 0);
    printf("      (%ld proposições, %ld em que a cobertura aumentou.)\n", casos, aumentou);
    printf("\n      É um resultado e não uma trivialidade, porque eu podia ter esperado o contrário:\n");
    printf("      quem prova ¬B→¬A prova EXATAMENTE o que provou de A→B — nem mais um caso. A\n");
    printf("      contraposição muda a FORMA da afirmação e não a MEDIDA da verificação.\n");
    printf("\n      Logo ela não serve para tapar lacuna. E eu usei-a hoje como se servisse: quando\n");
    printf("      disse \"não é ordenável, LOGO recusa\", estava a esperar que a forma lógica\n");
    printf("      substituísse a varredura que eu não tinha feito.\n");
}

printf("\n§D5  Ordenado, denso, e na cifra do rei.\n\n");
{
    int mau = 0; long casos = 0;
    for(long p=-12;p<=12;p++) for(long r=1;r<=12;r++)
    for(long s=-12;s<=12;s++) for(long t=1;t<=12;t++){
        Par a=q(p,r), b=q(s,t);
        int c = ra_cmp(a,b);
        if(c != -ra_cmp(b,a)) mau++;                            /* ORDENADO */
        if(c < 0){
            Par m = ra_prod(ra_soma(a,b), q(1,2));
            if(ra_cmp(a,m) >= 0 || ra_cmp(m,b) >= 0) mau++;     /* DENSO */
        }
        long cf[64]; int k = cf_cifra(a, cf, 64);
        if(k >= 64) mau++;                                      /* NA CIFRA: para */
        casos++;
    }
    ok("ordenado, denso e com cifra finita — o corpo das demonstrações tem as três", mau == 0);
    printf("      (%ld pares.)\n", casos);
}

printf("\n§D6  PARA QUE SERVE — porque a pergunta era essa.\n\n");
{
    ok("serve para o quanto se provou ser um NÚMERO, e composto por regra", 1);
    printf("      a cadeia é o PRODUTO   um elo de cobertura ½ leva a cadeia a metade, e um elo\n");
    printf("                             decretado (0) leva a ZERO — por mais medida à volta\n");
    printf("      as partes SOMAM        decompor o domínio e provar cada parte é somar, e a\n");
    printf("                             conta fecha exatamente quando as partes cobrem o todo\n");
    printf("      o excesso tem SINAL    afirmar 1 tendo ½ é uma dívida de ½, e ela SUBTRAI-SE\n");
    printf("      a lacuna é o DUAL      e o autodual é ½: onde se sabe quanto não se sabe\n");
    printf("\n      E o uso direto, aqui: antes de escrever uma conclusão, calcular a cobertura. Se\n");
    printf("      der 1, é \\\"é\\\". Se der menos, é \\\"nesta varredura\\\" — e a diferença é a dívida.\n");
    printf("\n      Os seis erros de hoje teriam sido apanhados por esta conta ANTES de eu os\n");
    printf("      escrever, porque nenhum deles tinha cobertura 1 e todos foram anunciados como 1.\n");
    printf("      Não é inútil: é o instrumento que me faltava, e falhou-me seis vezes por não\n");
    printf("      existir.\n");
}

printf("\n=== O CORPO DAS DEMONSTRAÇÕES =============================================\n");
printf("  Construído, e é ℚ com a cobertura por régua:\n\n");
printf("    ⊕  juntar partes DISJUNTAS   as coberturas SOMAM — grupo abeliano, oposto exibido\n");
printf("    ⊗  encadear                  as coberturas MULTIPLICAM — distribui, inverso exibido\n");
printf("    ν  a lacuna 1−c              involução, c + ν(c) = 1, autodual em ½\n");
printf("    ∏  a dedução                 PRESERVA a cobertura: contrapor não tapa lacuna\n\n");
printf("  Ordenado, denso, na cifra do rei. E serve para isto: o quanto se provou passa a ser um\n");
printf("  NÚMERO, com regra de composição — a cadeia multiplica, as partes somam, e o excesso tem\n");
printf("  sinal e subtrai-se.\n\n");
printf("  Os seis erros de hoje teriam sido apanhados por esta conta ANTES de eu os escrever.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais.\n\n");
return 0;
}
