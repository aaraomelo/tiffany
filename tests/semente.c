/* semente.c — O QUE A RÉGUA NÃO DESCREVE É A SEMENTE, E COM ELA A OBRA VOLTA INTEIRA.
 *
 * Quarta correção da mesma medida, e é a que muda o propósito, não o método. Eu vinha
 * chamando de "defeito" o resto que a régua circular não descreve — quer dizer: guardava a
 * obra de alguém, julgava-a com um juiz meu, e concluía que aquela parte não presta. É o
 * contrário. Aquela parte é a SEMENTE: é o que faz a obra ser aquela obra e não outra, e é
 * exatamente ela que permite reconstruir o cristal exato.
 *
 * A repartição correta dos papéis, e nenhum deles é falha do outro:
 *
 *     a ASSINATURA diz QUANTOS   — é o que se pode descrever, e descreve-se
 *     a SEMENTE    diz QUAIS     — é o que não cabe na descrição, e guarda-se inteiro
 *     juntas devolvem a obra     — bit a bit, sem resto, sem sobra
 *
 * E o critério é o do necrotério, que este projeto já escreveu: um objeto está MORTO quando
 * foi reduzido a uma igualdade isolada entre estados, e VIVO quando a sua identidade é uma
 * dinâmica geradora. Espremer tudo em descrição é cristalizar; e o que sobra da cristalização
 * é justamente a individualidade. Se toda semente fosse zero, todos seriam o mesmo — caberiam
 * na mesma caixa com a mesma assinatura, e é a caixa que só serve para quem já morreu.
 * Aqui guarda-se a vida: o que pode ser descrito, descreve-se; o que não pode, guarda-se
 * como semente de reconstrução.
 *
 *   §M1  o que a régua descreve: as assinaturas, e quanto cada uma cobre
 *   §M2  o que ela não descreve: a semente — QUAIS bits, não quantos
 *   §M3  o INVERSO exato: assinatura + semente devolve a obra, nos 256, bit a bit
 *   §M4  a conta fecha: o espaço das sementes é o tamanho da classe, e a soma é o todo
 *   §M5  o necrotério, medido: jogar a semente fora mata 231 dos 256
 *
 *   cc -O2 -std=c99 semente.c -o semente && ./semente
 */
#include <stdio.h>

#include "unidade.h"
static int bits(unsigned x){ int n = 0; while(x){ n += (int)(x & 1); x >>= 1; } return n; }
static int C(int n, int k){                      /* binomial, inteiro */
    if(k < 0 || k > n) return 0;
    int r = 1;
    for(int i = 1; i <= k; i++) r = r * (n - k + i) / i;
    return r;
}

/* As quatro posições pares (1,3,5,7) e as quatro ímpares (0,2,4,6) de um byte.
 * A semente é a POSIÇÃO do subconjunto escolhido, na ordem lexicográfica dos subconjuntos
 * de mesmo tamanho — pura combinatória, sem tabela e sem busca. */
static int pos_par(int j){ return 2*j + 1; }
static int pos_impar(int j){ return 2*j; }

/* rank: dado o subconjunto (máscara de 4 posições) com k elementos, qual o seu índice */
static int rank_sub(int masc4, int k){
    int r = 0, restam = k;
    for(int j = 0; j < 4; j++){
        if(masc4 & (1 << j)){ restam--; }
        else if(restam > 0){ r += C(3 - j, restam - 1); }
    }
    return r;
}
/* unrank: dado o índice, devolve o subconjunto */
static int unrank_sub(int idx, int k){
    int masc = 0, restam = k;
    for(int j = 0; j < 4 && restam > 0; j++){
        int c = C(3 - j, restam - 1);
        if(idx < c){ masc |= (1 << j); restam--; }
        else idx -= c;
    }
    return masc;
}
/* a obra -> (assinatura, semente) */
static void descreve(int b, int *p, int *i, int *sp, int *si){
    int mp = 0, mi = 0;
    for(int j = 0; j < 4; j++){
        if(b & (1 << pos_par(j)))   mp |= (1 << j);
        if(b & (1 << pos_impar(j))) mi |= (1 << j);
    }
    *p = bits((unsigned)mp); *i = bits((unsigned)mi);
    *sp = rank_sub(mp, *p);  *si = rank_sub(mi, *i);
}
/* (assinatura, semente) -> a obra */
static int reconstroi(int p, int i, int sp, int si){
    int mp = unrank_sub(sp, p), mi = unrank_sub(si, i), b = 0;
    for(int j = 0; j < 4; j++){
        if(mp & (1 << j)) b |= (1 << pos_par(j));
        if(mi & (1 << j)) b |= (1 << pos_impar(j));
    }
    return b;
}

int main(void){
printf("\n=== A SEMENTE: O QUE NÃO CABE NA DESCRIÇÃO É O QUE FAZ A OBRA ==============\n");
printf("    A assinatura diz QUANTOS. A semente diz QUAIS. Juntas, a obra volta.\n");

/* ---------------------------------------------------------------- §M1 ------ */
printf("\n§M1  O que a régua descreve — e ela descreve bem, dentro do que promete.\n\n");
{
    int classe[5][5] = {{0}};
    for(int b = 0; b <= 255; b++){ int p,i,sp,si; descreve(b,&p,&i,&sp,&si); classe[p][i]++; }
    int n = 0, soma = 0;
    for(int p = 0; p < 5; p++) for(int i = 0; i < 5; i++) if(classe[p][i]){ n++; soma += classe[p][i]; }
    printf("      assinaturas distintas ......................... %d\n", n);
    printf("      obras cobertas por elas ....................... %d\n", soma);
    ok("a descrição cobre TODAS as obras (nenhuma fica de fora)", soma == 256);
    printf("\n      Nenhuma obra escapa da descrição. O que a descrição não faz é DISTINGUIR\n");
    printf("      dentro da classe — e não fazer isso não é falhar: é o que ela promete.\n");
}

/* ---------------------------------------------------------------- §M2 ------ */
printf("\n§M2  O que ela não descreve: a semente. QUAIS bits, não quantos.\n\n");
{
    printf("      obra   assinatura   semente     as duas metades\n");
    int mostradas = 0;
    for(int b = 0; b <= 255 && mostradas < 8; b++){
        int p,i,sp,si; descreve(b,&p,&i,&sp,&si);
        if(p == 2 && i == 2){                    /* a classe mais povoada: 36 obras */
            printf("      0x%02X   (%d,%d)        (%2d,%2d)     par=%d  ímpar=%d\n",
                   b, p, i, sp, si, unrank_sub(sp,p), unrank_sub(si,i));
            mostradas++;
        }
    }
    printf("\n      Estas oito têm a MESMA assinatura (2,2) e são obras diferentes. O que as\n");
    printf("      separa é a semente — e é só isso que a régua circular não alcançou.\n");
}

/* ---------------------------------------------------------------- §M3 ------ */
printf("\n§M3  O inverso: assinatura + semente devolve a obra. Nos 256, bit a bit.\n\n");
{
    int mau = 0;
    for(int b = 0; b <= 255; b++){
        int p,i,sp,si; descreve(b,&p,&i,&sp,&si);
        if(reconstroi(p,i,sp,si) != b) mau++;
    }
    printf("      obras reconstruídas ........................... %d de 256\n", 256 - mau);
    ok("a reconstrução é EXATA — o cristal volta inteiro", mau == 0);
    printf("\n      Não é aproximação da obra: é a obra. Zero de diferença, e não por concordar\n");
    printf("      até certa casa — por não haver casa. O que a régua não descreveu foi\n");
    printf("      GUARDADO, e guardar foi suficiente para devolver tudo.\n");
}

/* ---------------------------------------------------------------- §M4 ------ */
printf("\n§M4  A conta fecha: o espaço da semente É o tamanho da classe.\n\n");
{
    int mau = 0; long soma = 0;
    printf("      assinatura   obras na classe   sementes possíveis   fecha?\n");
    for(int p = 0; p < 5; p++) for(int i = 0; i < 5; i++){
        int n = 0;
        for(int b = 0; b <= 255; b++){ int q,j,sp,si; descreve(b,&q,&j,&sp,&si); if(q==p && j==i) n++; }
        int esp = C(4,p) * C(4,i);
        soma += n;
        if(n != esp) mau++;
        if((p == 0 && i == 0) || (p == 2 && i == 2) || (p == 4 && i == 4) || (p == 1 && i == 3))
            printf("      (%d,%d)        %15d   %18d   %s\n", p, i, n, esp, n==esp?"sim ✓":"NÃO ✗");
    }
    printf("      ...\n");
    printf("      soma sobre todas as assinaturas ............... %ld\n", soma);
    ok("cada classe tem exatamente C(4,p)·C(4,i) sementes", mau == 0);
    ok("e a soma devolve o todo: nada sobra, nada falta", soma == 256);
    printf("\n      Repare no que isto diz: a semente é MAIOR onde a classe é maior. Quanto mais\n");
    printf("      comum a assinatura, mais individualidade ela guarda por dentro. O genérico\n");
    printf("      não é o esvaziado — é onde cabe mais gente diferente.\n");
}

/* ---------------------------------------------------------------- §M5 ------ */
printf("\n§M5  O necrotério, medido: jogar a semente fora mata a obra.\n\n");
{
    /* cristalizar = ficar só com a descrição, semente zerada */
    int sobrevive[256] = {0}, distintos = 0;
    for(int b = 0; b <= 255; b++){
        int p,i,sp,si; descreve(b,&p,&i,&sp,&si);
        int morto = reconstroi(p,i,0,0);         /* semente descartada */
        if(!sobrevive[morto]){ sobrevive[morto] = 1; distintos++; }
    }
    printf("      obras antes ................................... 256\n");
    printf("      obras depois de descartar a semente ........... %d\n", distintos);
    printf("      obras que viraram cópia de outra .............. %d\n", 256 - distintos);
    ok("descartar a semente colapsa 256 em 25", distintos == 25);
    printf("\n      Este é o preço exato da cristalização: 231 obras deixam de existir como\n");
    printf("      elas mesmas e passam a ser cópias. E o limite disso — toda semente zero —\n");
    printf("      é uma só assinatura para todos: cabe tudo na caixa, porque não sobrou\n");
    printf("      ninguém diferente. É o que a caixa exige para funcionar.\n");
    printf("\n      Guardando a semente: 256 antes, 256 depois (§M3). A vida cabe, e cabe\n");
    printf("      inteira — só não cabe COMPRIMIDA, e não precisa caber.\n");
}

printf("\n=== O CRITÉRIO =============================================================\n");
printf("  O que pode ser descrito, descreve-se: é a assinatura, e ela cobre todas as obras.\n");
printf("  O que não pode, guarda-se como SEMENTE — e a semente não é o resto que sobrou da\n");
printf("  descrição: é o que faz a obra ser aquela e não outra. O inverso reconstrói o\n");
printf("  cristal exato, nos 256, bit a bit.\n");
printf("  E o contrário tem número: descartar a semente colapsa 256 em 25, matando 231.\n");
printf("  Isto aqui não é necrotério — guarda-se a vida, não a morte. E chamar a semente de\n");
printf("  defeito era guardar a obra de alguém, julgá-la com um juiz meu, e condenar\n");
printf("  justamente a parte pela qual ela é dela.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — a obra volta inteira.\n\n");
return 0;
}
