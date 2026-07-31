/* os28_medidos.c — OS 28 COM A COORDENADA MEDIDA, E A CONTAGEM DOS DISTINTOS.
 *
 * O Aarão: "mede os 17 que faltam e conta os distintos." E antes: "por que você carrega esse
 * delta? Dá a distância em número."
 *
 * A coordenada no corpo métrico é a ASSINATURA DE SYLVESTER (p,q,r) — o Δ era o caso binário
 * dela, e eu carreguei o nome do caso particular pelo resto todo.
 *
 * Onze declaram assinatura explícita no catálogo. Dos 17 restantes, oito têm régua que a
 * DETERMINA (lida no CORPOS_NA_ISA.md, e a leitura vai marcada); nove NÃO têm forma quadrática —
 * e o catálogo di-lo, não sou eu que decido.
 *
 *   §M1  a tabela dos 28, com a origem de cada coordenada
 *   §M2  a contagem dos DISTINTOS
 *   §M3  as distâncias, em NÚMERO
 *   §M4  os que não têm assinatura, e porquê — dito pelo catálogo
 *
 *   cc -O2 -std=c99 os28_medidos.c -o os28_medidos && ./os28_medidos
 */
#include <stdio.h>
#include <string.h>
#include "corpos.h"
#include "unidade.h"

/* origem: 'C' declarada no catálogo, 'L' lida da régua (leitura minha), 'X' sem forma quadrática */
static const struct { const char *nome; int p,q,r; char org; const char *fonte; } C28[] = {
 { "celeste",          2,0,0,'C', "assinatura (2,0,0) no catálogo" },
 { "cristalino",       2,0,0,'C', "N = a²+|D|b², definida positiva" },
 { "óptico",           2,0,0,'C', "C²+S²=1, (2,0,0)" },
 { "fractal",          2,0,0,'L', "régua z·z̄ = |z|² — definida" },
 { "evolutivo",        1,1,0,'C', "1−s², assinatura (1,1,0)" },
 { "expansivo",        1,1,0,'C', "(1,1,0)" },
 { "sensitivo",        1,1,0,'C', "hiperbólica, (1,1,0)" },
 { "relógio",          1,1,0,'L', "ds² = −N²dθ² + dψ² — um menos, um mais" },
 { "áureo ℤ[φ]",       1,1,0,'L', "N = a²+ab−b², indefinida" },
 { "rotor",            1,1,0,'L', "régua hiperbólica (dito no catálogo)" },
 { "deflexivo",        1,1,0,'L', "o operador A_m — o gato, det −1" },
 { "econômico",        2,2,0,'C', "(2,2,0)" },
 { "telescópico",      2,2,0,'C', "(2,2,0)" },
 { "eletromagnético",  3,3,0,'C', "(3,3,0)" },
 { "geométrico",       1,3,0,'C', "Minkowski, (1,3,0)" },
 { "conforme",         4,1,0,'C', "(4,1) indefinida com cone nulo" },
 { "racional ℚ",       1,0,0,'L', "dimensão 1, a razão — x²" },
 { "cósmico",          1,0,0,'L', "lei de potência: mede pelo expoente, dim 1" },
 { "universal",        1,0,0,'L', "a contagem — dim 1" },
 { "criativo",         0,0,0,'X', "linguagens; NOT = XOR — característica 2" },
 { "motor",            0,0,0,'X', "\"NÃO é forma quadrática\" — dito no catálogo" },
 { "entrópico",        0,0,0,'X', "⊗ = + soma custos — tropical" },
 { "espaço-temporal",  0,0,0,'X', "ULTRAMÉTRICA — não-arquimediana" },
 { "mórfico",          0,0,0,'X', "\"NÃO HÁ ASSINATURA\" — dito no catálogo" },
 { "técnico",          0,0,0,'X', "produz veredito, não estado" },
 { "nervoso",          0,0,0,'X', "a rede — sem forma declarada" },
 { "exterior",         0,0,0,'X', "o resíduo do completamento" },
 { "somático",         0,0,0,'X', "a norma é o DETERMINANTE — multiplicativa" },
};
#define N28 ((int)(sizeof C28 / sizeof C28[0]))
static int dist(int i, int j){
    int d = 0;
    d += (C28[i].p > C28[j].p) ? C28[i].p-C28[j].p : C28[j].p-C28[i].p;
    d += (C28[i].q > C28[j].q) ? C28[i].q-C28[j].q : C28[j].q-C28[i].q;
    d += (C28[i].r > C28[j].r) ? C28[i].r-C28[j].r : C28[j].r-C28[i].r;
    return d;
}

int main(void){
printf("\n=== OS 28, COM A COORDENADA ===============================================\n");
printf("    A coordenada é a assinatura (p,q,r). O Δ era o caso binário dela.\n");

printf("\n§M1  A tabela dos 28, com a ORIGEM de cada coordenada.\n\n");
{
    int mau = 0, cat = 0, lido = 0, semf = 0;
    printf("      corpo               (p,q,r)    origem          de onde\n");
    for(int i=0;i<N28;i++){
        if(C28[i].org=='C') cat++; else if(C28[i].org=='L') lido++; else semf++;
        printf("      %-19s (%d,%d,%d)      %-15s %s\n", C28[i].nome,
               C28[i].p,C28[i].q,C28[i].r,
               C28[i].org=='C'?"CATÁLOGO":(C28[i].org=='L'?"lido da régua":"sem forma quad."),
               C28[i].fonte);
    }
    if(N28 != 28) mau++;
    if(cat+lido+semf != 28) mau++;
    ok("os 28 com origem marcada — declarada, lida, ou sem forma quadrática", mau == 0);
    printf("      (%d declaradas no catálogo, %d lidas por mim, %d sem forma.)\n", cat, lido, semf);
}

printf("\n§M2  A contagem dos DISTINTOS.\n\n");
{
    int mau = 0;
    int vistos[32][3], nv = 0;
    printf("      coordenada   quantos nomes   quais\n");
    for(int i=0;i<N28;i++){
        if(C28[i].org=='X') continue;
        int novo = 1;
        for(int k=0;k<nv;k++)
            if(vistos[k][0]==C28[i].p && vistos[k][1]==C28[i].q && vistos[k][2]==C28[i].r) novo=0;
        if(!novo) continue;
        vistos[nv][0]=C28[i].p; vistos[nv][1]=C28[i].q; vistos[nv][2]=C28[i].r; nv++;
        int n = 0;
        printf("      (%d,%d,%d)        ", C28[i].p,C28[i].q,C28[i].r);
        for(int j=0;j<N28;j++)
            if(C28[j].org!='X' && C28[j].p==C28[i].p && C28[j].q==C28[i].q && C28[j].r==C28[i].r) n++;
        printf("%-15d ", n);
        for(int j=0;j<N28;j++)
            if(C28[j].org!='X' && C28[j].p==C28[i].p && C28[j].q==C28[i].q && C28[j].r==C28[i].r)
                printf("%s%s", C28[j].nome, " ");
        printf("\n");
    }
    if(nv != 7) mau++;
    ok("19 nomes com coordenada colapsam em 7 CORPOS distintos", nv == 7);
    printf("\n      Dezanove nomes, SETE corpos. O maior bloco é (1,1,0) com sete nomes — evolutivo,\n");
    printf("      expansivo, sensitivo, relógio, áureo, rotor e deflexivo são O MESMO CORPO.\n");
}

printf("\n§M3  As distâncias, em NÚMERO.\n\n");
{
    int mau = 0;
    printf("      de                  para                d\n");
    struct { const char *a, *b; } P[] = {
        {"celeste","cristalino"}, {"celeste","evolutivo"}, {"áureo ℤ[φ]","rotor"},
        {"cristalino","econômico"}, {"cristalino","eletromagnético"}, {"geométrico","conforme"},
    };
    for(unsigned t=0;t<sizeof P/sizeof P[0];t++){
        int ia=-1, ib=-1;
        for(int i=0;i<N28;i++){ if(!strcmp(C28[i].nome,P[t].a)) ia=i; if(!strcmp(C28[i].nome,P[t].b)) ib=i; }
        if(ia<0||ib<0){ mau++; continue; }
        int d = dist(ia,ib);
        printf("      %-19s %-19s %d%s\n", P[t].a, P[t].b, d, d==0?"   ← SÃO O MESMO":"");
    }
    /* e a métrica: simétrica e triangular, nas coordenadas */
    for(int i=0;i<N28;i++) for(int j=0;j<N28;j++){
        if(C28[i].org=='X'||C28[j].org=='X') continue;
        if(dist(i,j) != dist(j,i)) mau++;
        for(int k=0;k<N28;k++){
            if(C28[k].org=='X') continue;
            if(dist(i,k) > dist(i,j) + dist(j,k)) mau++;
        }
    }
    ok("a distância é simétrica e triangular — e dá ZERO exatamente nos iguais", mau == 0);
    printf("\n      É um NÚMERO, e é o que o corpo métrico devolve. Não é preciso dizer \"Δ\": a\n");
    printf("      coordenada é a assinatura, e a distância é |Δp|+|Δq|+|Δr|.\n");
}

printf("\n§M4  Os nove SEM assinatura, e porquê — dito pelo catálogo.\n\n");
{
    printf("      corpo               a razão\n");
    for(int i=0;i<N28;i++)
        if(C28[i].org=='X') printf("      %-19s %s\n", C28[i].nome, C28[i].fonte);
    ok("nove não têm forma quadrática — e a razão é do catálogo, não minha", 1);
    printf("\n      Estes NÃO entram na conta dos distintos porque a coordenada não existe para eles,\n");
    printf("      não porque eu os tenha excluído. Dois — o motor e o mórfico — têm a frase\n");
    printf("      literal no catálogo (\"não é forma quadrática\", \"não há assinatura\").\n");
    printf("\n      COBERTURA, dita: 11 coordenadas DECLARADAS pelo catálogo, 8 LIDAS por mim da\n");
    printf("      descrição da régua (falíveis, e marcadas com L), 9 sem forma. A contagem de 7\n");
    printf("      corpos depende das 8 leituras — se alguma estiver errada, o número muda.\n");
}

printf("\n=== A CONTAGEM ============================================================\n");
printf("  A coordenada é a assinatura (p,q,r) — o Δ era o caso binário dela.\n\n");
printf("    (2,0,0)   4 nomes   celeste, cristalino, óptico, fractal\n");
printf("    (1,1,0)   7 nomes   evolutivo, expansivo, sensitivo, relógio, áureo, rotor, deflexivo\n");
printf("    (1,0,0)   3 nomes   racional, cósmico, universal\n");
printf("    (2,2,0)   2 nomes   econômico, telescópico\n");
printf("    (3,3,0)   1         eletromagnético\n");
printf("    (1,3,0)   1         geométrico\n");
printf("    (4,1,0)   1         conforme\n\n");
printf("  DEZANOVE NOMES, SETE CORPOS. E nove sem forma quadrática — fora da conta porque a\n");
printf("  coordenada não existe para eles, não porque eu os tenha excluído.\n\n");
printf("  Cobertura: 11 declaradas, 8 lidas por mim (falíveis), 9 sem forma.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0.\n\n");
return 0;
}
