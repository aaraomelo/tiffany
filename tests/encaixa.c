/* encaixa.c — A CIFRA DO ESPAÇO SEMÂNTICO: a base sai dos PRÓPRIOS vetores.
 *
 * O Aarão: "é só encaixar os embeddings na cifra — é teletransporte, segue protocolo. Se o
 * espaço dele tem n vetores, acho que n+1 vetores formam a base. Essa é a cifra. Só normalizar.
 * A base dele é ortonormal."
 *
 * E ISTO CORRIGE O QUE EU IA FAZER. A minha primeira versão cifrava os embeddings com o TELÓMERO
 * — o hash de Euclides que serve para o texto e para os pesos. Funcionava como identificador e
 * falhava no que interessa: um hash espalha, e espalhar destrói a vizinhança, que é a única
 * coisa que um embedding tem de útil.
 *
 * A cifra do espaço semântico é outra, e sai de dentro dele: se o espaço tem n vetores,
 * \(n{+}1\) determinam-no, e a base é a ortonormalização deles. Não se IMPÕE uma base de fora —
 * o `semantico.c` §S4 usa Hadamard, que é legítimo e fecha a 1,1e-15, mas Hadamard vem de fora.
 * Aqui a base é a dos próprios dados, e a pergunta que decide é se ela já é quase ortogonal —
 * porque se for, "só normalizar" basta, e é isso que o Aarão está a dizer.
 *
 *   §C1  o ESPAÇO: n vetores, e quantos são precisos para o gerar
 *   §C2  JÁ SÃO QUASE ORTOGONAIS? — a concentração de medida em alta dimensão
 *   §C3  a BASE por Gram-Schmidt: ortonormal, e mede-se que é
 *   §C4  a CIFRA: projetar na base e voltar — o protocolo, com resíduo
 *   §C5  e ela PRESERVA a vizinhança — o que o telómero não fazia
 *
 *   cc -O2 -std=c99 -I. encaixa.c -lm -o encaixa && ./encaixa [/tmp/emb.txt]
 */
#include <stdio.h>
#include "../lib/disco.h"
#define v DISCO_FIXO2(double, ND, 82)
#define base DISCO_FIXO2(double, ND, 83)

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "le_emb.h"          /* le os dois formatos: 0x… (bits) e decimal */
#include "unidade.h"

#define NF 24
#define ND 1024

static char  nome[NF][64];

static int nf = 0, nd = 0;

static double dot(const double *a, const double *b, int n){
    double s = 0;
    for(int i = 0; i < n; i++) s += a[i]*b[i];
    return s;
}

int main(int argc, char **argv){
    disco_prende(DISCO_BASE(82),"dados/v.bin",(size_t)(NF)*(ND),sizeof(double));
    disco_zera(v,(size_t)(NF)*(ND),sizeof(double));
    disco_prende(DISCO_BASE(83),"dados/base.bin",(size_t)(NF)*(ND),sizeof(double));
    disco_zera(base,(size_t)(NF)*(ND),sizeof(double));
const char *fich = argc > 1 ? argv[1] : "/tmp/emb.txt";
FILE *f = fopen(fich, "r");
if(!f){ printf("\nencaixa: sem %s — corre tools/colhe_emb.sh\n", fich); return 1; }
{
    char *lin = DISCO_FIXO(char, 220);
    disco_prende(DISCO_BASE(220),"dados/lin_220.bin",(size_t)(65536),sizeof(char)); disco_zera(lin,(size_t)(65536),sizeof(char));
    while(fgets(lin, ((size_t)(65536)*sizeof(char)), f) && nf < NF){
        char *tab = strchr(lin, '\t');
        if(!tab) continue;
        *tab = 0;
        snprintf(nome[nf], sizeof nome[nf], "%.60s", lin);
        int d = 0;
        for(char *p = strtok(tab+1, " \n"); p && d < ND; p = strtok(NULL, " \n"))
            { char *fim_; v[nf][d++] = emb_le(p, &fim_); }
        if(!nd) nd = d;
        nf++;
    }
    fclose(f);
}
printf("\n=== A CIFRA DO ESPAÇO SEMÂNTICO: A BASE SAI DOS PRÓPRIOS VETORES =========\n");
printf("    %d vetores, %d dimensões — do nomic-embed-text, colhidos por colhe_emb.sh\n", nf, nd);

printf("\n§C1  O ESPAÇO: quantos vetores são precisos para o gerar.\n\n");
{
    /* Se o espaco tem n vetores, n+1 determinam-no — e' o simplex. Mas o que se mede aqui e' o
     * POSTO real: quantos dos n sao linearmente independentes. Se forem todos, o espaco que
     * eles geram tem dimensao n, e n+1 seria um a mais — que e' exatamente o que fecha. */
    double (*m)[ND] = DISCO_FIXO2(double, ND, 170);
    disco_prende(DISCO_BASE(170),"dados/enc_m.bin",(size_t)(NF)*(ND),sizeof(double));
    memcpy(m, v, ((size_t)(NF)*(ND)*sizeof(double)));
    int posto = 0;
    for(int i = 0; i < nf; i++){
        for(int j = 0; j < posto; j++){
            double c = dot(m[i], m[j], nd) / dot(m[j], m[j], nd);
            for(int d = 0; d < nd; d++) m[i][d] -= c*m[j][d];
        }
        double n2 = dot(m[i], m[i], nd);
        if(n2 > 1e-12){
            if(posto != i) memcpy(m[posto], m[i], sizeof m[i]);
            posto++;
        }
    }
    printf("      vetores colhidos            %d\n", nf);
    printf("      dimensão do espaço          %d\n", nd);
    printf("      posto (independentes)       %d\n", posto);
    printf("      logo geram um subespaço de dimensão %d, e %d+1 = %d o determinam\n\n",
           posto, posto, posto+1);
    ok("os vetores são linearmente independentes — nenhum é combinação dos outros",
       posto == nf);
    printf("      Em 768 dimensões, %d vetores quaisquer serem independentes não é sorte: é o\n", nf);
    printf("      que se espera quando há muito mais eixos do que vetores. O espaço é largo,\n");
    printf("      e é essa largura que faz o resto funcionar.\n");
}

printf("\n§C2  JÁ SÃO QUASE ORTOGONAIS? — e é isto que autoriza o \"só normalizar\".\n\n");
{
    /* A afirmacao do Aarao — "so normalizar" — so' vale se os vetores ja' vierem quase
     * ortogonais. Em alta dimensao isso e' esperado por concentracao de medida: dois vetores
     * ao acaso em R^768 tem cosseno ~ 1/sqrt(768) = 0,036. Mede-se o cosseno REAL entre pares
     * e compara-se com esse valor — que nao e' regua minha, e' o que a dimensao impoe. */
    double pior = 0, soma = 0; int n = 0;
    for(int i = 0; i < nf; i++) for(int j = i+1; j < nf; j++){
        double c = dot(v[i],v[j],nd) / sqrt(dot(v[i],v[i],nd)*dot(v[j],v[j],nd));
        if(fabs(c) > pior) pior = fabs(c);
        soma += fabs(c); n++;
    }
    double media = soma/n, esperado = 1.0/sqrt((double)nd);
    printf("      cosseno médio entre pares        %.4f\n", media);
    printf("      pior caso                        %.4f\n", pior);
    printf("      o acaso em %d dimensões daria    %.4f\n\n", nd, esperado);
    ok("os vetores NÃO são ortogonais — o cosseno médio excede muito o do acaso",
       media > 3*esperado);
    printf("      E aqui a medida diz o contrário do que \"só normalizar\" sugere: estes vetores\n");
    printf("      têm cosseno médio %.2f, muito acima dos %.3f do acaso. Não é defeito — é o\n", media, esperado);
    printf("      SIGNIFICADO: 'rei' e 'rainha' não são ortogonais porque não são independentes.\n");
    printf("      Normalizar arruma o comprimento e não arruma o ângulo, portanto sozinho não\n");
    printf("      dá base ortonormal. É preciso ortogonalizar — e é o que o §C3 faz.\n");
}

printf("\n§C3  A BASE por Gram-Schmidt: e mede-se que ela É ortonormal.\n\n");
{
    /* Gram-Schmidt: tira-se de cada vetor o que ele tem dos anteriores, e normaliza-se. O que
     * se mede nao e' que o algoritmo correu — e' que o RESULTADO tem as duas propriedades:
     * norma 1 em cada, e produto interno 0 entre quaisquer dois. */
    for(int i = 0; i < nf; i++){
        memcpy(base[i], v[i], (size_t)nd*sizeof(double));
        for(int j = 0; j < i; j++){
            double c = dot(base[i], base[j], nd);
            for(int d = 0; d < nd; d++) base[i][d] -= c*base[j][d];
        }
        double nn = sqrt(dot(base[i], base[i], nd));
        if(nn > 1e-12) for(int d = 0; d < nd; d++) base[i][d] /= nn;
    }
    double pior_norma = 0, pior_orto = 0;
    for(int i = 0; i < nf; i++){
        double e = fabs(sqrt(dot(base[i],base[i],nd)) - 1.0);
        if(e > pior_norma) pior_norma = e;
        for(int j = i+1; j < nf; j++){
            double c = fabs(dot(base[i],base[j],nd));
            if(c > pior_orto) pior_orto = c;
        }
    }
    printf("      pior desvio da norma 1           %.3e\n", pior_norma);
    printf("      pior produto interno entre pares %.3e\n\n", pior_orto);
    ok("cada vetor da base tem norma 1", pior_norma < 1e-12);
    ok("e são ortogonais dois a dois — a base É ortonormal", pior_orto < 1e-12);
    printf("      A base sai dos DADOS, não de fora. O semantico.c §S4 usa Hadamard, que fecha\n");
    printf("      igualmente bem e vem de fora; esta vem de dentro, e é isso que a torna a\n");
    printf("      cifra DESTE espaço e não de um espaço qualquer.\n");
}

printf("\n§C4  A CIFRA: projetar na base e voltar — o protocolo, com resíduo.\n\n");
{
    /* O protocolo do teletransporte.c: entra (projeta-se), atravessa, volta (recompoe-se), e
     * mede-se o residuo. Como a base e' ortonormal e gera o subespaco dos vetores, a volta tem
     * de ser EXATA — e exata quer dizer na casa do epsilon, nao "quase". */
    printf("      vetor            ‖v‖        ‖v reconstruído‖   resíduo relativo\n");
    double pior = 0;
    for(int i = 0; i < nf; i++){
        double coef[NF];
        for(int k = 0; k < nf; k++) coef[k] = dot(v[i], base[k], nd);   /* a cifra */
        double *volta = DISCO_FIXO(double, 90);
        disco_prende(DISCO_BASE(90),"dados/volta_90.bin",(size_t)((ND)),sizeof(double));
        disco_zera(volta,(size_t)((ND)),sizeof(double));
        for(int d = 0; d < nd; d++) volta[d] = 0;
        for(int k = 0; k < nf; k++)
            for(int d = 0; d < nd; d++) volta[d] += coef[k]*base[k][d];
        double e = 0, den = 0;
        for(int d = 0; d < nd; d++){ double t = volta[d]-v[i][d]; e += t*t; den += v[i][d]*v[i][d]; }
        double rel = sqrt(e/den);
        if(rel > pior) pior = rel;
        if(i < 4) printf("      %-16s %-10.4f %-18.4f %.3e\n",
                         nome[i], sqrt(den), sqrt(dot(volta,volta,nd)), rel);
    }
    printf("      …\n\n      pior resíduo relativo: %.3e\n\n", pior);
    ok("o vetor cifrado na base própria volta EXATO — resíduo na casa do epsilon",
       pior < 1e-12);
    printf("      %d coeficientes bastam para guardar um vetor de %d dimensões, e a volta é\n", nf, nd);
    printf("      exata — porque a base gera exatamente o subespaço onde os vetores vivem.\n");
    printf("      É compressão sem perda, e não por sorte: por ortonormalidade.\n");
}

printf("\n§C5  E ELA PRESERVA A VIZINHANÇA — o que o telómero não fazia.\n\n");
{
    /* A diferenca que decide entre esta cifra e o telomero. Um hash espalha; uma base
     * ORTONORMAL preserva distancias — e' Parseval, e mede-se: a distancia entre dois vetores
     * tem de ser IGUAL a distancia entre os seus coeficientes. Se isto falhasse, a cifra
     * servia para identificar e nao para procurar. */
    double pior = 0;
    for(int i = 0; i < nf; i++) for(int j = i+1; j < nf; j++){
        double ci[NF], cj[NF];
        for(int k = 0; k < nf; k++){ ci[k] = dot(v[i],base[k],nd); cj[k] = dot(v[j],base[k],nd); }
        double dv = 0, dc = 0;
        for(int d = 0; d < nd; d++){ double t = v[i][d]-v[j][d]; dv += t*t; }
        for(int k = 0; k < nf; k++){ double t = ci[k]-cj[k]; dc += t*t; }
        double e = fabs(sqrt(dv) - sqrt(dc)) / sqrt(dv);
        if(e > pior) pior = e;
    }
    printf("      pior desvio entre a distância no espaço e a distância na cifra: %.3e\n\n", pior);
    ok("a cifra preserva as distâncias — Parseval, e é o que a torna boa para procurar",
       pior < 1e-12);
    printf("      É aqui que esta cifra e o telómero se separam, e cada uma serve para o que a\n");
    printf("      outra não serve: o telómero ESPALHA (por isso identifica sem colidir) e esta\n");
    printf("      PRESERVA (por isso procura). Não são duas versões da mesma coisa — são o par\n");
    printf("      dual, e a busca precisa das duas, como o dna.c §N6 já dizia.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    A base sai dos próprios vetores e é ortonormal por construção; a volta é\n");
printf("    exata; e as distâncias sobrevivem. Mas 'só normalizar' não bastava — os\n");
printf("    vetores têm cosseno médio muito acima do acaso, e isso é o significado.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
