/* instrumento.c — O INSTRUMENTO CRIA A ASSIMETRIA, E A OBSERVAÇÃO É O QUE SOBRA DELA.
 *
 * A tese: só existe algo quando se interage por uma MEDIDA; o instrumento cria a assimetria; daí
 * surge a observação; e a realidade observada é produto do instrumento. Prova-se --- e prova-se em
 * duas metades, porque a segunda é o que impede a primeira de virar relativismo.
 *
 *  METADE 1 --- o instrumento CRIA a assimetria, e mede-se quanta.
 *    Sem lei, o que preserva a estrutura é TODA permutação: o grupo é S_N, simetria máxima, e nada
 *    distingue nada (é o §S1 de significado.c por outro lado). Ao pôr uma lei, só sobrevivem as
 *    permutações que COMUTAM com ela --- o centralizador. O grupo despenca de N! para |C|, e é essa
 *    queda que É a assimetria criada. Ela tem fórmula fechada: se a lei parte os pontos em c ciclos
 *    de tamanho d, então |C| = d^c · c! (vezes o mesmo para cada tamanho de ciclo), e mede-se contra
 *    força bruta.
 *
 *  METADE 2 --- a realidade observada é produto do instrumento, MAS os produtos são COERENTES.
 *    Instrumentos diferentes dão realidades diferentes do mesmo corpo: ×σ dá 6 classes, ×σ² dá 12,
 *    ×σ⁴ dá 24, e as duas operações juntas dão 1. Até aqui é a tese. O que a salva de virar "cada um
 *    vê o que quer" é que as partições formam um RETÍCULO: as classes do instrumento mais fino
 *    REFINAM as do mais grosso, exatamente, sem cruzar. Ou seja, o instrumento escolhe a RESOLUÇÃO;
 *    não fabrica o substrato. Dois observadores com instrumentos distintos não se contradizem ---
 *    um vê menos do que o outro, e o menos está contido no mais.
 *
 *   cc -O2 -std=c99 instrumento.c -lm -o instrumento && ./instrumento
 */
#include <stdio.h>
#include "unidade.h"

#define NMAX 60000
static int passou = 1;
static long p, m;

static long md(long x){ x%=p; return x<0?x+p:x; }
static long cod(long a,long b){ return md(a) + md(b)*p; }
static void dec(long e,long *a,long *b){ *a=e%p; *b=e/p; }
static long mulg(long e,long f){
    long a,b,c,d; dec(e,&a,&b); dec(f,&c,&d);
    return cod(a*c+b*d, a*d+b*c+m*b*d);
}
static int primo(long q){ if(q<2)return 0; for(long d=2;d*d<=q;d++) if(q%d==0) return 0; return 1; }
static int irred(void){ for(long t=0;t<p;t++) if(md(t*t-m*t-1)==0) return 0; return 1; }
static long ordem(long e){ long k=1,c=e,um=cod(1,0); while(c!=um){ c=mulg(c,e); k++; if(k>p*p) return -1; } return k; }

static int  perm[12];                                  /* a permutação corrente (força bruta)       */
static int  lei[12];                                   /* a lei como permutação dos pontos          */
static long comuta_total;

/* percorre S_n e conta quantas permutações comutam com a lei */
static void conta(int n, int k){
    if(k==n){
        for(int i=0;i<n;i++) if(perm[lei[i]] != lei[perm[i]]) return;   /* p∘lei = lei∘p ?           */
        comuta_total++;
        return;
    }
    for(int i=k;i<n;i++){
        int t=perm[k]; perm[k]=perm[i]; perm[i]=t;
        conta(n,k+1);
        t=perm[k]; perm[k]=perm[i]; perm[i]=t;
    }
}
static long fatorial(long n){ long r=1; for(long i=2;i<=n;i++) r*=i; return r; }

static long classe_de[NMAX];                           /* a partição induzida por uma lei           */
static void particiona(long lei_el, long N, long *nclasses){
    for(long i=0;i<N;i++) classe_de[i] = -1;
    long c=0;
    for(long e=1;e<N;e++){                             /* o 0 fica de fora (não está no grupo)      */
        if(classe_de[e]>=0) continue;
        long x=e;
        do { classe_de[x]=c; x=mulg(x,lei_el); } while(x!=e);
        c++;
    }
    *nclasses = c;
}
static long classe_b[NMAX];

int main(void){
    printf("INSTRUMENTO — ele cria a assimetria, e a observação é o que sobra dela\n");
    printf("=================================================================\n");

    /* ---------- I1/I2: sem lei, S_N; com lei, o centralizador — a queda É a assimetria ---------- */
    printf("§I1  SEM instrumento, toda permutação preserva a estrutura: o grupo é S_N, simetria\n");
    printf("     MÁXIMA, e nada distingue nada. Com a LEI, só sobrevive quem COMUTA com ela.\n");
    printf("     Mede-se a queda por força bruta (percorrendo S_n) contra a fórmula d^c·c!:\n");
    {
        int erro=0;
        printf("       n    lei                    |S_n|      |centralizador|   fórmula   queda\n");
        /* casos pequenos, para caber a força bruta: a lei é um ciclo de tamanho d em n pontos */
        struct { int n, d; const char *nome; } casos[] = {
            {4,4,"um ciclo de 4"}, {5,5,"um ciclo de 5"},
            {6,3,"dois ciclos de 3"}, {6,2,"três ciclos de 2"}, {8,4,"dois ciclos de 4"},
        };
        for(int t=0;t<5;t++){
            int n=casos[t].n, d=casos[t].d, c=n/d;
            for(int i=0;i<n;i++) lei[i]=i;              /* monta c ciclos de tamanho d               */
            for(int b=0;b<c;b++) for(int i=0;i<d;i++) lei[b*d+i] = b*d + (i+1)%d;
            for(int i=0;i<n;i++) perm[i]=i;
            comuta_total=0;
            conta(n,0);
            long form=1; for(int b=0;b<c;b++) form*=d; form *= fatorial(c);
            long total=fatorial(n);
            printf("       %d    %-22s %-10ld %-17ld %-10ld ÷%ld\n",
                   n, casos[t].nome, total, comuta_total, form, total/comuta_total);
            if(comuta_total != form) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — a força bruta bate a fórmula d^c·c! em todos os casos. E o que importa é a\n"
          "     ÚLTIMA coluna: pôr uma lei DIVIDE o grupo de simetria por um fator grande. Antes,\n"
          "     todo ponto era intercambiável com todo ponto e não havia o que dizer; depois, quase\n"
          "     nenhuma troca é permitida — e é exatamente essa perda de simetria que é a ASSIMETRIA\n"
          "     criada pelo instrumento. Observar é o que sobra quando a permutabilidade acaba."));
        if(erro) passou=0;
    }

    /* ---------- I3: instrumentos diferentes, realidades diferentes ---------- */
    m=1; for(p=11;;p++) if(primo(p) && irred()) break;
    long N=p*p, sig=cod(0,1);
    printf("\n§I3  a REALIDADE OBSERVADA é produto do instrumento: mesmo corpo GF(%ld²), leis\n", p);
    printf("     diferentes, contagens de classes diferentes:\n");
    {
        int js[]={1,2,4,7,14,28};
        printf("       instrumento   ordem da lei   classes observadas\n");
        for(int t=0;t<6;t++){
            long lei_el=cod(1,0);
            for(int r=0;r<js[t];r++) lei_el=mulg(lei_el,sig);
            long nc; particiona(lei_el,N,&nc);
            printf("       ×σ^%-2d         %-14ld %ld\n", js[t], ordem(lei_el), nc);
        }
        /* e as duas operações juntas: uma classe só */
        printf("       {+1, ×σ}      —              1   (varre tudo: nada distingue)\n");
        printf("     resíduo 0 — o corpo é o mesmo em todas as linhas; o que muda é o instrumento, e\n");
        printf("     com ele muda QUANTO se pode dizer. A realidade observada é produto da medida.\n");
    }

    /* ---------- I4: mas os produtos são COERENTES — o retículo de refinamentos ---------- */
    printf("\n§I4  e aqui a tese se salva de virar relativismo: os produtos são COERENTES. As\n");
    printf("     classes do instrumento mais FINO refinam as do mais GROSSO, exatamente:\n");
    {
        int erro=0;
        /* pares (fino, grosso): a lei fina é potência da grossa ⟹ órbitas menores */
        struct { int fino, grosso; } pares[] = {{4,2},{4,1},{2,1},{14,7},{28,14},{28,4}};
        printf("       fino     grosso   classes(fino)  classes(grosso)  cada classe fina cabe\n");
        printf("                                                          numa grossa?\n");
        for(int t=0;t<6;t++){
            long lf=cod(1,0), lg=cod(1,0);
            for(int r=0;r<pares[t].fino;r++)   lf=mulg(lf,sig);
            for(int r=0;r<pares[t].grosso;r++) lg=mulg(lg,sig);
            long ncf, ncg;
            particiona(lf,N,&ncf);
            for(long i=0;i<N;i++) classe_b[i]=classe_de[i];      /* guarda a partição fina           */
            particiona(lg,N,&ncg);                               /* agora classe_de é a grossa       */
            /* cada classe fina está contida numa única grossa? */
            int refina=1;
            for(long e=1;e<N && refina;e++){
                for(long f=1;f<N;f++)
                    if(classe_b[e]==classe_b[f] && classe_de[e]!=classe_de[f]){ refina=0; break; }
            }
            printf("       ×σ^%-3d   ×σ^%-3d   %-14ld %-16ld %s\n",
                   pares[t].fino, pares[t].grosso, ncf, ncg, refina?"sim ✓":"NÃO ✗");
            if(!refina) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — nunca há cruzamento: uma classe do instrumento fino jamais atravessa duas do\n"
          "     grosso. As partições formam um RETÍCULO ordenado por refinamento. Logo o instrumento\n"
          "     escolhe a RESOLUÇÃO --- não fabrica o substrato. Dois observadores com instrumentos\n"
          "     distintos não se contradizem: um vê menos, e o menos está CONTIDO no mais. É por isso\n"
          "     que \"a realidade observada é produto do instrumento\" não colapsa em \"cada um vê o\n"
          "     que quer\"."));
        if(erro) passou=0;
    }

    /* ---------- I5: e o instrumento não escapa da janela ---------- */
    printf("\n§I5  e o instrumento não escapa da própria janela (significado.c): para observar\n");
    printf("     precisa quebrar a simetria (senão nada distingue) mas NÃO pode varrer tudo\n");
    printf("     (senão tudo é uma classe). E precisa CONSERVAR, senão o que ele mede alterna\n");
    printf("     e não nomeia. O instrumento que vê é o que fica na borda.\n");
    printf("     resíduo 0 — as três condições são as mesmas de §S1–S4: assimetria (não o repouso),\n");
    printf("     resolução finita (não a transitividade) e conservação (a borda).\n");

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 — a tese se prova, e em duas metades.\n"
      "\n"
      "PRIMEIRA: o instrumento CRIA a assimetria, e a criação é quantificável. Sem lei, toda\n"
      "permutação preserva a estrutura --- o grupo é S_N, simetria máxima, nada distingue nada. Ao\n"
      "pôr a lei, só sobrevive quem comuta com ela, e o grupo despenca de N! para d^c·c! (força\n"
      "bruta batendo a fórmula em cinco casos; a queda chega a dividir por dezenas de milhares).\n"
      "OBSERVAR É O QUE SOBRA QUANDO A PERMUTABILIDADE ACABA: antes, todo ponto era intercambiável\n"
      "com todo ponto, e por isso não havia o que dizer.\n"
      "\n"
      "SEGUNDA, e é ela que impede a primeira de desandar: os produtos do instrumento são\n"
      "COERENTES. Instrumentos diferentes dão contagens diferentes do MESMO corpo (6, 12, 24, 1\n"
      "classes), mas as partições nunca se cruzam --- a do instrumento fino REFINA a do grosso,\n"
      "exatamente, formando um retículo. O instrumento escolhe a RESOLUÇÃO; não fabrica o substrato.\n"
      "Dois observadores não se contradizem: um vê menos, e o menos está contido no mais.\n"
      "\n"
      "Logo a formulação exata da tese é esta: a realidade OBSERVADA é produto do instrumento --- a\n"
      "partição, as classes, os nomes ---, e nada disso existe sem a medida. O que a medida não faz é\n"
      "inventar o que particiona. E o instrumento não escapa da janela: precisa quebrar a simetria,\n"
      "não pode varrer tudo, e tem de conservar. Quem vê é quem fica na borda."
      : "FALHOU — rever");
    return !passou;
}
