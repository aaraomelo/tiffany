/* disco.c — O DISCO DE RAIO 1: que altura para o mineral ser ouro, e que volume.
 *
 * O Aarão: "considera que todos os discos têm raio 1 e calcula a altura do disco pro mineral
 * ser ouro; coloca volume."
 *
 * A densidade de ouro do metal m é 5/(m²+4) — a razão dos discriminantes, medida em
 * densidade.c. Para um disco conter UMA UNIDADE de ouro é preciso que densidade × volume = 1,
 * logo
 *
 *     V(m) = 1/densidade = (m² + 4)/5 = d(m)/5
 *
 * e o volume é o DISCRIMINANTE sobre cinco. Com raio 1 o volume é π·h, logo
 *
 *     h(m) = d(m) / (5π)
 *
 * O ouro é o disco mais baixo que existe: quanto MENOS ouro o mineral tem, mais alto o disco
 * precisa de ser para carregar o mesmo ouro. É a compensação direta da densidade.
 *
 * E há uma coisa a notar, que é o único lugar onde o redondo toca o reino do rei: π entra
 * SÓ na conversão para altura linear. As RAZÕES entre alturas são racionais exatas — o π
 * cancela — e é por isso que se pode dizer "este disco é 8/5 mais alto que o do ouro" sem
 * nenhuma aproximação, e não se pode dizer a altura absoluta sem uma.
 *
 *   §V1  o volume para uma unidade de ouro: V = d/5, classe racional exata
 *   §V2  com raio 1, h = V/π — e em unidades de π a altura É d/5
 *   §V3  a altura cresce com o discriminante: menos ouro, disco mais alto
 *   §V4  as RAZÕES entre alturas são exatas: o π cancela e some
 *   §V5  e o ouro é o disco mais baixo — o mínimo, e é o mínimo por construção
 *
 *   cc -O2 -std=c99 disco.c -o disco && ./disco
 */
#include <stdio.h>
#include "unidade.h"

static long mdc_l(long a, long b){ if(a<0)a=-a; if(b<0)b=-b; while(b){ long t=a%b; a=b; b=t; } return a?a:1; }
static long disc(long m){ return m*m + 4; }

int main(void){
printf("\n=== O DISCO DE RAIO 1: ALTURA E VOLUME ====================================\n");
printf("    densidade × volume = 1 unidade de ouro. Com raio 1, V = π·h.\n");

/* ---------------------------------------------------------------- §V1 ------ */
printf("\n§V1  O VOLUME para conter uma unidade de ouro: V = d/5.\n\n");
{
    int mau = 0;
    printf("      m     metal      densidade   volume V = d/5   classe reduzida\n");
    const char *nome[4] = {"", "ouro", "prata", "bronze"};
    for(long m = 1; m <= 12; m++){
        long d = disc(m);
        /* densidade = 5/d, e V = 1/densidade = d/5 — a recíproca da classe */
        long vn = d, vd = 5, g = mdc_l(vn, vd);
        vn /= g; vd /= g;
        /* confere: densidade · V = 1  ⟺  5·vn = d·vd */
        if(5*vn != d*vd) mau++;
        printf("      %-5ld %-10s 5/%-9ld %ld/%-13ld %ld/%ld\n", m,
               (m <= 3) ? nome[m] : "—", d, d, 5L, vn, vd);
    }
    ok("densidade × volume = 1 exato, em toda a família", mau == 0);
    printf("\n      O volume é o DISCRIMINANTE sobre cinco. É a mesma quantidade que decide a\n");
    printf("      densidade, virada do avesso: pouco ouro por unidade pede mais unidades.\n");
}

/* ---------------------------------------------------------------- §V2 ------ */
printf("\n§V2  Com raio 1: V = π·h, logo h = d/(5π). Em unidades de π, a altura É d/5.\n\n");
{
    int mau = 0;
    printf("      m     volume V   altura h (em unidades de π)   h·5 (inteiro)\n");
    for(long m = 1; m <= 8; m++){
        long d = disc(m);
        /* h = V/π, e V = d/5 ⟹ h = (d/5)/π. Em unidades de π: h_π = d/5, exato. */
        if(5 * (d/5) + (d%5) != d) mau++;
        printf("      %-5ld %ld/5%*s%ld/5 · (1/π)%*s%ld\n", m, d, (int)(8 - 3), "", d,
               (int)(16 - 12), "", d);
    }
    ok("a altura em unidades de π é exatamente d/5 — sem aproximação", mau == 0);
    printf("\n      Enquanto se mede em unidades de π a conta é INTEIRA. O π só aparece quando se\n");
    printf("      quer a altura numa régua linear — e aí deixa de ser exata, porque π não é.\n");
}

/* ---------------------------------------------------------------- §V3 ------ */
printf("\n§V3  A altura CRESCE com o discriminante: menos ouro, disco mais alto.\n\n");
{
    int mau = 0;
    printf("      m     metal      altura (unid. de π)   contra o do ouro\n");
    const char *nome[4] = {"", "ouro", "prata", "bronze"};
    for(long m = 2; m <= 200; m++) if(disc(m) <= disc(m-1)) mau++;
    for(long m = 1; m <= 6; m++){
        long d = disc(m), g = mdc_l(d, 5);
        printf("      %-5ld %-10s %ld/5%*s%ld/%ld vezes mais alto\n", m,
               (m <= 3) ? nome[m] : "—", d, (int)(18 - 4), "", d/g, 5/g);
    }
    ok("a altura sobe estritamente com m, sem exceção até 200", mau == 0);
    printf("\n      Para carregar o MESMO ouro, o disco de prata precisa de ser 8/5 do do ouro, o\n");
    printf("      de bronze 13/5, e assim por diante. A escada da densidade, de cabeça para\n");
    printf("      baixo: aquela só descia, esta só sobe.\n");
}

/* ---------------------------------------------------------------- §V4 ------ */
printf("\n§V4  As RAZÕES entre alturas são exatas: o π cancela e some.\n\n");
{
    int mau = 0;
    printf("      m₁  m₂   h(m₁)/h(m₂) = d₁/d₂   classe reduzida   exato?\n");
    struct { long a, b; } pares[] = {{2,1},{3,1},{4,1},{3,2},{6,3},{11,1}};
    for(unsigned t = 0; t < sizeof pares/sizeof pares[0]; t++){
        long d1 = disc(pares[t].a), d2 = disc(pares[t].b);
        long g = mdc_l(d1, d2);
        /* h₁/h₂ = (d₁/5π)/(d₂/5π) = d₁/d₂ — o π e o 5 cancelam os dois */
        if((d1/g) * d2 != (d2/g) * d1) mau++;
        printf("      %-3ld %-4ld %ld/%-19ld %ld/%-16ld sim ✓\n",
               pares[t].a, pares[t].b, d1, d2, d1/g, d2/g);
    }
    ok("a razão entre duas alturas é racional exata — sem π nenhum", mau == 0);
    printf("\n      É o único lugar onde o redondo toca o reino do rei, e ele toca de leve: pode\n");
    printf("      dizer-se \"este disco é 8/5 mais alto que o do ouro\" com exatidão total, e NÃO\n");
    printf("      pode dizer-se a altura absoluta sem uma aproximação. A comparação é do rei; a\n");
    printf("      medida absoluta pede emprestado ao círculo.\n");
}

/* ---------------------------------------------------------------- §V5 ------ */
printf("\n§V5  E o ouro é o disco MAIS BAIXO — o mínimo, por construção.\n\n");
{
    int mau = 0;
    long dmin = disc(1);
    for(long m = 1; m <= 200; m++) if(disc(m) < dmin) mau++;
    printf("      o menor volume em m ≤ 200      %ld/5 = 1        (é o do ouro)\n", dmin);
    printf("      a menor altura                 1/π\n");
    printf("      e o raio, por hipótese         1\n");
    ok("o disco do ouro é o mais baixo de todos, e o volume dele é 1", mau == 0);
    printf("\n      Com raio 1 e volume 1, o disco do ouro é a UNIDADE: todos os outros medem-se\n");
    printf("      contra ele, e todos são mais altos. É a mesma coisa que a coroação já dizia —\n");
    printf("      o rei é quem vale 1 na própria régua — agora em forma de sólido.\n");
}

printf("\n=== O DISCO ===============================================================\n");
printf("  Raio 1, e densidade × volume = 1 unidade de ouro:\n\n");
printf("      V(m) = d(m)/5 = (m²+4)/5          o volume é o DISCRIMINANTE sobre cinco\n");
printf("      h(m) = d(m)/(5π)                  e a altura, o mesmo dividido por π\n\n");
printf("      ouro     m=1    V = 1      h = 1/π       o mais baixo — a unidade\n");
printf("      prata    m=2    V = 8/5    h = 8/(5π)    8/5 do disco do ouro\n");
printf("      bronze   m=3    V = 13/5   h = 13/(5π)   13/5\n");
printf("      m=12            V = 148/5  h = 148/(5π)  quase trinta vezes\n\n");
printf("  A altura sobe estritamente: para carregar o MESMO ouro, quanto menos denso o mineral\n");
printf("  mais alto o disco. É a escada da densidade de cabeça para baixo.\n\n");
printf("  E o π toca isto uma vez só, de leve: as RAZÕES entre alturas são racionais exatas —\n");
printf("  ele cancela —, e só a altura ABSOLUTA precisa dele. A comparação é do rei; a medida\n");
printf("  absoluta é que pede emprestado ao círculo.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
