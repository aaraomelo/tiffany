/* racional_pg.c — A PG PARA AS OPERAÇÕES RACIONAIS SOBRE CLASSES INTEIRAS.
 *
 * O Aarão: "PA para operações sequenciais e PG para operações racionais sobre classes inteiras
 * — dessa forma você tem o banco de dados."
 *
 * A PA já entrou: a varredura é uma progressão no endereço, e o bytecode deixou de crescer com
 * a tabela. Falta a outra metade, e ela é o lado multiplicativo.
 *
 * E a peça já estava na ISA sem eu ver: a Word tem DUAS componentes, {total, e}. Um racional é
 * um par. Então o dado do banco não precisa de estrutura nova nenhuma — o numerador mora no
 * total e o denominador no e, e a mesma palavra que já circulava passa a carregar uma fração.
 *
 * As duas operações são as do projeto, e nenhuma delas divide:
 *
 *     ⊗ La Hire    (a,b)·(c,d) = (a·c, b·d)          componente a componente
 *     ⊕ Clifford   (a,b)+(c,d) = (a·d + b·c, b·d)    cruzado
 *
 * E a CLASSE é o que importa: (a,b) e (ka,kb) são o mesmo número. Reduzir pelo mdc dá o
 * representante, e ele é único. É por isso que se fala em "classes inteiras" — o objeto não é
 * o par, é a classe dele.
 *
 * A PG entra aqui: multiplicar repetidamente por uma razão fixa É uma progressão geométrica, e
 * é essa a operação racional que o banco precisa (juro, proporção, escala). Cada passo vai e
 * volta EXATO, sem arredondar nada — mas até um TETO, que é o da palavra. Com p e q primos
 * entre si a redução não cancela nada, e o denominador cresce como q^k: com 101/100, um juro de
 * 1%, são quatro passos garantidos pela guarda conservadora. Isso não é defeito da conta, é o alcance dela — e o
 * medidor PARA antes de virar, em vez de virar calado, que foi o que a primeira versão fez.
 *
 *   §Q1  a classe: (a,b) ~ (ka,kb), e o representante reduzido é único
 *   §Q2  ⊗ La Hire fecha nas classes: a classe do produto só depende das classes
 *   §Q3  ⊕ Clifford idem — e as duas juntas distribuem
 *   §Q4  a comparação SEM DIVISÃO: a/b < c/d ⟺ a·d < c·b, e a ordem é consistente
 *   §Q5  a PG: exata até ao TETO da palavra — e o teto mede-se, não se supõe
 *
 *   cc -O2 -std=c99 racional_pg.c -o racional_pg && ./racional_pg
 */
#include <stdio.h>
#include "unidade.h"

typedef struct { long n, d; } Rac;        /* (total, e) da Word: numerador e denominador */

static long mdc(long a, long b){ if(a<0)a=-a; if(b<0)b=-b; while(b){ long t=a%b; a=b; b=t; } return a?a:1; }
/* o representante da classe: reduzido, e com o denominador sempre positivo */
static Rac classe(Rac x){
    if(x.d < 0){ x.n = -x.n; x.d = -x.d; }
    long g = mdc(x.n, x.d);
    x.n /= g; x.d /= g;
    return x;
}
static Rac lahire(Rac x, Rac y){ Rac r = { x.n*y.n, x.d*y.d }; return r; }          /* ⊗ */
static Rac clifford(Rac x, Rac y){ Rac r = { x.n*y.d + x.d*y.n, x.d*y.d }; return r; } /* ⊕ */
/* a comparação, sem dividir: com denominadores positivos, a/b < c/d ⟺ a·d < c·b */
static int compara(Rac x, Rac y){
    x = classe(x); y = classe(y);
    long e = x.n * y.d, f = y.n * x.d;
    return (e > f) - (e < f);
}

int main(void){
printf("\n=== A PG PARA AS OPERAÇÕES RACIONAIS ======================================\n");
printf("    A Word já tinha duas componentes. Um racional é um par: nada de novo a criar.\n");

/* ---------------------------------------------------------------- §Q1 ------ */
printf("\n§Q1  A CLASSE: (a,b) e (ka,kb) são o mesmo, e o reduzido é único.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      par        ×k        classe do primeiro   classe do segundo   iguais?\n");
    for(long a = -6; a <= 6; a++) for(long b = 1; b <= 8; b++)
    for(long k = -5; k <= 5; k++){
        if(k == 0) continue;
        Rac x = { a, b }, y = { k*a, k*b };
        Rac cx = classe(x), cy = classe(y);
        if(cx.n != cy.n || cx.d != cy.d) mau++;
        casos++;
        if((a==3&&b==6&&k==2)||(a==-4&&b==6&&k==-3))
            printf("      (%ld,%ld)%*s(%ld,%ld)%*s(%ld,%ld)%*s(%ld,%ld)%*ssim ✓\n",
                   a,b, 5,"", k*a,k*b, 3,"", cx.n,cx.d, 12,"", cy.n,cy.d, 10,"");
    }
    ok("escalar o par não muda a classe — o objeto é a classe", mau == 0);
    printf("      (%ld pares testados, com k negativo incluído.)\n", casos);
}

/* ---------------------------------------------------------------- §Q2 ------ */
printf("\n§Q2  ⊗ LA HIRE fecha nas classes: componente a componente, e nada mais.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      x        y        x⊗y          classe    e com x,y escalados dá o mesmo?\n");
    for(long a=-4;a<=4;a++) for(long b=1;b<=5;b++)
    for(long c=-4;c<=4;c++) for(long d=1;d<=5;d++)
    for(long k=1;k<=3;k++){
        Rac x={a,b}, y={c,d};
        Rac p1 = classe(lahire(x,y));
        Rac p2 = classe(lahire((Rac){k*a,k*b}, (Rac){2*c,2*d}));
        if(p1.n != p2.n || p1.d != p2.d) mau++;
        casos++;
        if(a==3&&b==4&&c==2&&d==5&&k==1)
            printf("      (%ld,%ld)%*s(%ld,%ld)%*s(%ld,%ld)%*s(%ld,%ld)%*ssim ✓\n",
                   a,b,3,"",c,d,3,"",a*c,b*d,5,"",p1.n,p1.d,4,"");
    }
    ok("o produto da classe só depende das classes — está bem definido", mau == 0);
    printf("      (%ld casos.)\n", casos);
    printf("\n      É a operação mais barata que existe: multiplicar dois pares é multiplicar as\n");
    printf("      componentes. Nada cruza, nada se soma, e não se divide em lugar nenhum.\n");
}

/* ---------------------------------------------------------------- §Q3 ------ */
printf("\n§Q3  ⊕ CLIFFORD idem — e as duas juntas distribuem.\n\n");
{
    int mau_b = 0, mau_d = 0; long casos = 0;
    for(long a=-3;a<=3;a++) for(long b=1;b<=4;b++)
    for(long c=-3;c<=3;c++) for(long d=1;d<=4;d++){
        Rac x={a,b}, y={c,d};
        Rac s1 = classe(clifford(x,y));
        Rac s2 = classe(clifford((Rac){3*a,3*b}, (Rac){2*c,2*d}));
        if(s1.n != s2.n || s1.d != s2.d) mau_b++;
        for(long e=-3;e<=3;e++){
            Rac z = {e, 2};
            Rac esq = classe(lahire(x, clifford(y,z)));
            Rac dir = classe(clifford(lahire(x,y), lahire(x,z)));
            if(esq.n != dir.n || esq.d != dir.d) mau_d++;
            casos++;
        }
    }
    ok("a soma da classe só depende das classes", mau_b == 0);
    ok("e ⊗ distribui sobre ⊕ — as duas fecham juntas", mau_d == 0);
    printf("      (%ld triplos na distributiva.)\n", casos);
}

/* ---------------------------------------------------------------- §Q4 ------ */
printf("\n§Q4  A COMPARAÇÃO SEM DIVISÃO: a/b < c/d ⟺ a·d < c·b.\n\n");
{
    int mau_t = 0, mau_x = 0; long casos = 0;
    for(long a=-5;a<=5;a++) for(long b=1;b<=4;b++)
    for(long c=-5;c<=5;c++) for(long d=1;d<=4;d++){
        Rac x={a,b}, y={c,d};
        int r = compara(x,y);
        /* tricotomia: exatamente um de <, =, > */
        int tri = (r==-1)+(r==0)+(r==1);
        if(tri != 1) mau_x++;
        /* antissimetria */
        if(compara(y,x) != -r) mau_x++;
        for(long e=-5;e<=5;e++){
            Rac z={e,3};
            if(compara(x,y) <= 0 && compara(y,z) <= 0 && compara(x,z) > 0) mau_t++;
            casos++;
        }
    }
    ok("tricotomia e antissimetria: a ordem é total", mau_x == 0);
    ok("e transitiva — em todos os triplos", mau_t == 0);
    printf("      (%ld triplos.)\n", casos);
    printf("\n      Nenhuma divisão em lugar nenhum: comparar duas frações é multiplicar cruzado\n");
    printf("      e comparar inteiros. A ISA não tem divisão, e não precisa de nenhuma.\n");
}

/* ---------------------------------------------------------------- §Q5 ------ */
printf("\n§Q5  A PG: exata até ao TETO da palavra — e o teto mede-se.\n\n");
{
    /* A exatidão é de graça enquanto os inteiros couberem. Com p e q primos entre si nada
     * cancela na redução, e o denominador cresce como q^k: o teto é log(2^63)/log(max(p,q)).
     * Aqui não se supõe — conta-se, parando ANTES de estourar em vez de estourar calado. */
    int mau = 0;
    printf("      razão      passos exatos até ao teto   o que estoura primeiro\n");
    struct { long p, q; } razoes[] = {{2,1},{3,2},{7,5},{101,100},{1,7},{10,9}};
    for(unsigned t = 0; t < sizeof razoes/sizeof razoes[0]; t++){
        Rac r = { razoes[t].p, razoes[t].q };
        Rac x = { 1, 1 };
        int k = 0;
        const long TETO = 3037000499L;          /* raiz de 2^63: além disto o produto pode virar */
        for(;;){
            if(x.n > TETO || x.n < -TETO || x.d > TETO) break;
            if(r.n && (x.n > TETO/(r.n<0?-r.n:r.n))) break;
            if(r.d && (x.d > TETO/r.d)) break;
            Rac y = classe(lahire(x, r));
            /* a volta tem de fechar em CADA passo, senão não é exato */
            Rac inv = { razoes[t].q, razoes[t].p };
            Rac v = classe(lahire(y, inv));
            if(v.n != x.n || v.d != x.d){ mau++; break; }
            x = y; k++;
            if(k > 200) break;
        }
        printf("      %ld/%-8ld %-25d %s\n", razoes[t].p, razoes[t].q, k,
               (razoes[t].p > razoes[t].q) ? "o numerador" : "o denominador");
    }
    ok("cada passo vai e volta exato — enquanto couber na palavra", mau == 0);
    printf("\n      A guarda é CONSERVADORA de propósito: para quando o valor passa da raiz de\n");
    printf("      2^63, e não quando o produto viraria de facto. Logo estes números são um piso\n");
    printf("      garantido, não o teto exato — o teto real é maior, e apurá-lo exigiria guarda\n");
    printf("      mais fina. Piso garantido é o que se pode prometer; teto exato, não.\n");
    printf("\n      E o piso já diz o essencial: com 101/100 — um juro de 1%% — são QUATRO passos\n");
    printf("      garantidos, porque 101 e 100 são primos entre si e a redução não cancela nada.\n");
    printf("      Passado o teto o inteiro vira, e vira em SILÊNCIO se ninguém olhar: foi\n");
    printf("      exatamente o que a primeira versão deste medidor fez, e o que a guarda impede.\n");
    printf("\n      O que isto diz ao banco: a operação racional é exata e reversível, e tem um\n");
    printf("      ALCANCE. Passar dele exige palavra maior ou arredondar de propósito — e\n");
    printf("      arredondar de propósito, com a regra dita, é honesto; estourar calado não é.\n");
}

printf("\n=== A PG PARA AS OPERAÇÕES RACIONAIS ======================================\n");
printf("    A Word já tinha duas componentes. Um racional é um par: nada de novo a criar.\n");

/* ---------------------------------------------------------------- §Q1 ------ */
printf("\n§Q1  A CLASSE: (a,b) e (ka,kb) são o mesmo, e o reduzido é único.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      par        ×k        classe do primeiro   classe do segundo   iguais?\n");
    for(long a = -6; a <= 6; a++) for(long b = 1; b <= 8; b++)
    for(long k = -5; k <= 5; k++){
        if(k == 0) continue;
        Rac x = { a, b }, y = { k*a, k*b };
        Rac cx = classe(x), cy = classe(y);
        if(cx.n != cy.n || cx.d != cy.d) mau++;
        casos++;
        if((a==3&&b==6&&k==2)||(a==-4&&b==6&&k==-3))
            printf("      (%ld,%ld)%*s(%ld,%ld)%*s(%ld,%ld)%*s(%ld,%ld)%*ssim ✓\n",
                   a,b, 5,"", k*a,k*b, 3,"", cx.n,cx.d, 12,"", cy.n,cy.d, 10,"");
    }
    ok("escalar o par não muda a classe — o objeto é a classe", mau == 0);
    printf("      (%ld pares testados, com k negativo incluído.)\n", casos);
}

/* ---------------------------------------------------------------- §Q2 ------ */
printf("\n§Q2  ⊗ LA HIRE fecha nas classes: componente a componente, e nada mais.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      x        y        x⊗y          classe    e com x,y escalados dá o mesmo?\n");
    for(long a=-4;a<=4;a++) for(long b=1;b<=5;b++)
    for(long c=-4;c<=4;c++) for(long d=1;d<=5;d++)
    for(long k=1;k<=3;k++){
        Rac x={a,b}, y={c,d};
        Rac p1 = classe(lahire(x,y));
        Rac p2 = classe(lahire((Rac){k*a,k*b}, (Rac){2*c,2*d}));
        if(p1.n != p2.n || p1.d != p2.d) mau++;
        casos++;
        if(a==3&&b==4&&c==2&&d==5&&k==1)
            printf("      (%ld,%ld)%*s(%ld,%ld)%*s(%ld,%ld)%*s(%ld,%ld)%*ssim ✓\n",
                   a,b,3,"",c,d,3,"",a*c,b*d,5,"",p1.n,p1.d,4,"");
    }
    ok("o produto da classe só depende das classes — está bem definido", mau == 0);
    printf("      (%ld casos.)\n", casos);
    printf("\n      É a operação mais barata que existe: multiplicar dois pares é multiplicar as\n");
    printf("      componentes. Nada cruza, nada se soma, e não se divide em lugar nenhum.\n");
}

/* ---------------------------------------------------------------- §Q3 ------ */
printf("\n§Q3  ⊕ CLIFFORD idem — e as duas juntas distribuem.\n\n");
{
    int mau_b = 0, mau_d = 0; long casos = 0;
    for(long a=-3;a<=3;a++) for(long b=1;b<=4;b++)
    for(long c=-3;c<=3;c++) for(long d=1;d<=4;d++){
        Rac x={a,b}, y={c,d};
        Rac s1 = classe(clifford(x,y));
        Rac s2 = classe(clifford((Rac){3*a,3*b}, (Rac){2*c,2*d}));
        if(s1.n != s2.n || s1.d != s2.d) mau_b++;
        for(long e=-3;e<=3;e++){
            Rac z = {e, 2};
            Rac esq = classe(lahire(x, clifford(y,z)));
            Rac dir = classe(clifford(lahire(x,y), lahire(x,z)));
            if(esq.n != dir.n || esq.d != dir.d) mau_d++;
            casos++;
        }
    }
    ok("a soma da classe só depende das classes", mau_b == 0);
    ok("e ⊗ distribui sobre ⊕ — as duas fecham juntas", mau_d == 0);
    printf("      (%ld triplos na distributiva.)\n", casos);
}

/* ---------------------------------------------------------------- §Q4 ------ */
printf("\n§Q4  A COMPARAÇÃO SEM DIVISÃO: a/b < c/d ⟺ a·d < c·b.\n\n");
{
    int mau_t = 0, mau_x = 0; long casos = 0;
    for(long a=-5;a<=5;a++) for(long b=1;b<=4;b++)
    for(long c=-5;c<=5;c++) for(long d=1;d<=4;d++){
        Rac x={a,b}, y={c,d};
        int r = compara(x,y);
        /* tricotomia: exatamente um de <, =, > */
        int tri = (r==-1)+(r==0)+(r==1);
        if(tri != 1) mau_x++;
        /* antissimetria */
        if(compara(y,x) != -r) mau_x++;
        for(long e=-5;e<=5;e++){
            Rac z={e,3};
            if(compara(x,y) <= 0 && compara(y,z) <= 0 && compara(x,z) > 0) mau_t++;
            casos++;
        }
    }
    ok("tricotomia e antissimetria: a ordem é total", mau_x == 0);
    ok("e transitiva — em todos os triplos", mau_t == 0);
    printf("      (%ld triplos.)\n", casos);
    printf("\n      Nenhuma divisão em lugar nenhum: comparar duas frações é multiplicar cruzado\n");
    printf("      e comparar inteiros. A ISA não tem divisão, e não precisa de nenhuma.\n");
}

printf("\n=== A PG PARA AS OPERAÇÕES RACIONAIS ======================================\n");
printf("  A Word da ISA já tinha duas componentes, e um racional é um par: o numerador no\n");
printf("  total, o denominador no e. Nada de estrutura nova — a mesma palavra que já circulava\n");
printf("  passa a carregar uma fração.\n\n");
printf("    ⊗ La Hire    (a,b)·(c,d) = (a·c, b·d)          componente a componente\n");
printf("    ⊕ Clifford   (a,b)+(c,d) = (a·d + b·c, b·d)    cruzado\n");
printf("    ordem        a/b < c/d ⟺ a·d < c·b             sem uma única divisão\n");
printf("    classe       (a,b) ~ (ka,kb), reduzido é único\n\n");
printf("  E a PG é a operação racional que o banco precisa — razão fixa, muitos passos: juro,\n");
printf("  proporção, escala. Cada passo vai e volta EXATO, sem perder um bit — mas até um TETO,\n");
printf("  que é o da palavra. Com 101/100 são quatro passos GARANTIDOS pela guarda conservadora\n");
printf("  (o teto real é maior; o piso é o que se promete), porque 101 e 100 são primos entre si\n");
printf("  e a redução não cancela nada.\n\n");
printf("  Isso não é defeito da conta: é o alcance dela, e o medidor para ANTES de virar em vez\n");
printf("  de virar calado — que foi o que a primeira versão fez.\n\n");
printf("  Com a PA no endereço e a PG no valor, o banco tem as duas: a sequencial e a racional.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
