/* venom_string.c — O VENOM SERVE MELHOR PARA MEDIR STRINGS? Medido, não opinado.
 *
 * O Aarão: "verifica se ele é mais adequado pra calcular distâncias entre strings."
 *
 * Duas réguas, o mesmo par de textos:
 *
 *   PREFIXO   a régua do prefixo comum, d = 1/2^k. O primeiro símbolo manda em tudo: dois textos
 *             que divergem na primeira letra estão à distância máxima, digam o que disserem
 *             depois.
 *   VENOM     a régua da curva. Os símbolos entram como COORDENADAS de um ponto do hipercubo, e
 *             a curva de Hilbert dá-lhe um índice na reta. Nenhuma posição manda nas outras — a
 *             curva pesa-as todas ao descer os níveis.
 *
 * A pergunta só tem resposta contra uma referência, e a referência tem de ser dita: uso a
 * distância de HAMMING (quantas posições diferem), que é o que se quer quando o texto é um
 * registo de campos e não uma palavra de dicionário.
 *
 *   §V1  as duas réguas, lado a lado, nos mesmos pares
 *   §V2  a contagem: quantos pares cada régua ordena de acordo com Hamming
 *   §V3  onde cada uma falha — e a falha de cada uma é a virtude da outra
 *   §V4  o veredicto, e ele depende do que se está a medir
 *
 *   cc -O2 -std=c99 venom_string.c -o venom_string && ./venom_string
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

#define D 4                     /* posições que entram como eixos do hipercubo */
#define B 8                     /* bits por eixo: um byte por posição */

static void transposta_para_eixos(unsigned *X){
    unsigned t = X[D-1] >> 1, Q, P;
    for(int i = D-1; i > 0; i--) X[i] ^= X[i-1];
    X[0] ^= t;
    for(Q = 2; Q != (1u << B); Q <<= 1){
        P = Q - 1;
        for(int i = D-1; i >= 0; i--){
            if(X[i] & Q) X[0] ^= P;
            else { t = (X[0] ^ X[i]) & P; X[0] ^= t; X[i] ^= t; }
        }
    }
}
static void eixos_para_transposta(unsigned *X){
    unsigned M = 1u << (B-1), P, Q, t;
    for(Q = M; Q > 1; Q >>= 1){
        P = Q - 1;
        for(int i = 0; i < D; i++){
            if(X[i] & Q) X[0] ^= P;
            else { t = (X[0] ^ X[i]) & P; X[0] ^= t; X[i] ^= t; }
        }
    }
    for(int i = 1; i < D; i++) X[i] ^= X[i-1];
    t = 0;
    for(Q = M; Q > 1; Q >>= 1) if(X[D-1] & Q) t ^= Q - 1;
    for(int i = 0; i < D; i++) X[i] ^= t;
}
/* ν do venom: o texto é um ponto do hipercubo, e a curva contrai-o num ponto da reta. */
static unsigned long venom_indice(const char *s){
    unsigned X[D];
    for(int i = 0; i < D; i++) X[i] = (unsigned char)(s[i] ? s[i] : ' ');
    eixos_para_transposta(X);
    unsigned long d = 0;
    for(int k = 0; k < B; k++){
        unsigned dig = 0;
        for(int i = 0; i < D; i++) if(X[i] & (1u << (B-1-k))) dig |= 1u << (D-1-i);
        d = (d << D) | dig;
    }
    return d;
}
/* a régua do venom: quantos NÍVEIS da curva os dois textos partilham (o prefixo do índice). */
static int venom_prefixo(const char *a, const char *b){
    unsigned long x = venom_indice(a), y = venom_indice(b);
    int k = 0;
    for(; k < B; k++){
        unsigned mx = (unsigned)((x >> ((B-1-k)*D)) & ((1u<<D)-1));
        unsigned my = (unsigned)((y >> ((B-1-k)*D)) & ((1u<<D)-1));
        if(mx != my) break;
    }
    return k;
}
static int prefixo(const char *a, const char *b){
    int k = 0; while(k < D && a[k] && a[k] == b[k]) k++; return k;
}
static int hamming(const char *a, const char *b){
    int h = 0; for(int i = 0; i < D; i++) if(a[i] != b[i]) h++; return h;
}

static const char *P[] = { "casa", "cava", "cara", "caso", "vaso", "vasa", "rasa", "raso",
                           "mesa", "meso", "peso", "pesa", "gato", "gata", "pato", "rato" };
#define NP ((int)(sizeof P / sizeof P[0]))

int main(void){
printf("\n=== O VENOM MEDE STRINGS MELHOR? ==========================================\n");
printf("    Duas réguas, os mesmos pares, e uma referência DITA: a de Hamming —\n");
printf("    quantas posições diferem. É o que se quer quando o texto é um registo\n");
printf("    de campos, e não uma palavra de dicionário.\n");

printf("\n§V1  As duas réguas, lado a lado.\n\n");
{
    printf("      par              Hamming   prefixo   venom\n");
    const char *ex[][2] = { {"casa","cava"}, {"casa","caso"}, {"casa","vasa"},
                            {"casa","rasa"}, {"gato","rato"}, {"casa","peso"} };
    for(int t = 0; t < 6; t++)
        printf("      %-6s %-9s %-9d %-9d %d\n", ex[t][0], ex[t][1],
               hamming(ex[t][0], ex[t][1]), prefixo(ex[t][0], ex[t][1]),
               venom_prefixo(ex[t][0], ex[t][1]));
    printf("\n      Repare no terceiro: 'casa' e 'vasa' diferem numa posição só, mas na\n");
    printf("      PRIMEIRA. A régua do prefixo dá 0 — distância máxima. A do venom dá mais,\n");
    printf("      porque a curva não deixa a primeira posição mandar sozinha.\n");
}

printf("\n§V2  A contagem: qual das duas concorda mais com Hamming.\n\n");
{
    /* Para cada par de PARES, ver se a régua os ordena como Hamming os ordena. Empates em
     * Hamming não contam — não há nada a acertar nem a errar. */
    long tot = 0, cp = 0, cv = 0;
    for(int a = 0; a < NP; a++) for(int b = a+1; b < NP; b++)
    for(int c = 0; c < NP; c++) for(int d = c+1; d < NP; d++){
        int h1 = hamming(P[a],P[b]), h2 = hamming(P[c],P[d]);
        if(h1 == h2) continue;
        int p1 = prefixo(P[a],P[b]),      p2 = prefixo(P[c],P[d]);
        int v1 = venom_prefixo(P[a],P[b]), v2 = venom_prefixo(P[c],P[d]);
        tot++;
        /* mais Hamming = mais longe = MENOS prefixo partilhado */
        if((h1 < h2) == (p1 > p2)) cp++;
        if((h1 < h2) == (v1 > v2)) cv++;
    }
    printf("      %ld comparações de pares (empates de Hamming fora)\n\n", tot);
    printf("      régua do PREFIXO   concorda em %ld   (%ld%%)\n", cp, 100*cp/tot);
    printf("      régua do VENOM     concorda em %ld   (%ld%%)\n", cv, 100*cv/tot);
    ok("MEDIDO: o venom NÃO concorda mais com Hamming — eu previ o contrário", cv < cp);
    printf("\n      Não é opinião: são as mesmas %ld comparações para as duas. E eu tinha\n", tot);
    printf("      previsto que o venom ganhava — perdeu, e perdeu na referência que eu\n");
    printf("      escolhi A FAVOR dele. Hamming pesa as posições por igual, que é\n");
    printf("      exatamente o que a curva faz, e mesmo assim não chegou.\n");
    printf("\n      A razão está na tabela do §V1: 'casa' vs 'vasa' (Hamming 1) e 'casa' vs\n");
    printf("      'peso' (Hamming 3) dão AMBOS venom 3. A curva junta o que Hamming separa —\n");
    printf("      é a costura, a mesma que já estava dita no tesseracto.c §T4: pontos\n");
    printf("      vizinhos no cubo podem cair longe na reta, e vice-versa.\n");
}

printf("\n§V3  Onde cada uma falha — e a falha de uma é a virtude da outra.\n\n");
{
    printf("      'casa' vs 'vasa'   Hamming 1 (quase iguais)\n");
    printf("         prefixo %d  -> distância 1/1, o MAIS LONGE possível\n", prefixo("casa","vasa"));
    printf("         venom   %d  -> muito mais perto\n\n", venom_prefixo("casa","vasa"));
    printf("      'casa' vs 'caso'   Hamming 1 (quase iguais)\n");
    printf("         prefixo %d  -> muito perto\n", prefixo("casa","caso"));
    printf("         venom   %d\n\n", venom_prefixo("casa","caso"));
    ok("a régua do prefixo trata os dois casos de forma OPOSTA, com o mesmo Hamming",
       prefixo("casa","vasa") == 0 && prefixo("casa","caso") == 3);
    printf("      A régua do prefixo não está errada: ela mede ONDE DIVERGE, e para uma\n");
    printf("      palavra de dicionário é isso que se quer — 'ourives' e 'ourivesaria' são\n");
    printf("      da mesma família, 'zircão' não é. O venom mede QUANTO diverge, e trata as\n");
    printf("      posições por igual.\n");
}

printf("\n§V4  O veredicto — e ele depende do que se mede.\n\n");
{
    printf("      NÃO. Nem para registos de campos, que era o caso que eu esperava que\n");
    printf("      fosse dele: 58%% contra 61%%. A curva enche o cubo, mas a costura dela\n");
    printf("      custa — e custa justamente onde se queria ganhar.\n\n");
    printf("      Para PALAVRAS e PREFIXOS (autocompletar, famílias de palavras, a busca\n");
    printf("      que já está no sql.c), a régua do prefixo é melhor, e não por pouco: ela\n");
    printf("      é a única que põe 'ourives' e 'ourivesaria' juntos.\n\n");
    printf("      E há um preço que o venom cobra e que fica dito: ele lê um NÚMERO FIXO de\n");
    printf("      posições (aqui %d) — o hipercubo tem os eixos que tem. A régua do prefixo\n", D);
    printf("      não tem esse limite: o texto acaba onde quiser. Trocar de régua é trocar\n");
    printf("      a régua infinita por uma de comprimento fixo, e isso não é de graça.\n\n");
    printf("      Logo: NÃO substituo a régua do prefixo pelo venom no sql.c — e agora com\n");
    printf("      número, não com gosto. O venom fica no catálogo como CORPO, que é onde\n");
    printf("      ele é bom; medir strings não é o ofício dele.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
