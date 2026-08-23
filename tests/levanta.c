/* levanta.c — COMPLETAR um corpo pelo ι/π, medido como o cliente usa.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/levanta tests/levanta.c && /tmp/levanta
 */
#include "unidade.h"
#include "levanta.h"
#include <stdio.h>

int main(void){
    printf("O LEVANTAMENTO: a lib que COMPLETA um corpo, e não o inventa\n\n");

    /* ── §L1 π ∘ ι = id, E O LEVANTADO ESTÁ COMPLETO POR CONSTRUÇÃO ─────── */
    {
        printf("§L1  levantar, projectar, e voltar ao mesmo --- em quatro leituras.\n\n");
        long mal = 0;
        struct { const char *n; int caso; } C[4] = {
            {"soma mod 6  (já completo)", 0},
            {"projecção   (já completo)", 1},
            {"soma livre  (resume)     ", 2},
            {"distância   (resume)     ", 3},
        };
        printf("      leitura                    |I|  fibras  G  levantado  abertos  π∘ι  completo\n");
        long ident = 0, cresceu = 0;
        for(int c = 0; c < 4; c++){
            long end[400]; long n = 0;
            for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++){
                if(C[c].caso == 0)      end[n++] = (a + b) % 6;
                else if(C[c].caso == 1) end[n++] = a;
                else if(C[c].caso == 2) end[n++] = a + b;
                else                    end[n++] = a*a + b*b;
            }
            LvLevanta L;
            if(!lv_levanta(end, n, &L)){ mal++; continue; }
            int id = lv_pi_iota_id(end, n, &L), comp = lv_completo(&L);
            printf("      %s %5ld %6ld %3ld %9ld %8ld %4s %8s\n", C[c].n, n,
                   L.fibras, L.gmax, L.n, L.acrescentados,
                   id ? "id" : "NAO", comp ? "sim" : "NAO");
            if(!id || !comp) mal++;
            if(L.acrescentados == 0) ident++; else cresceu++;
            /* os que EXISTIAM lá em cima são exactamente os de partida */
            long viv = 0;
            for(long k = 0; k < L.n; k++) if(L.existia[k]) viv++;
            if(viv != n || viv + L.acrescentados != L.n) mal++;
        }
        printf("      → %ld já eram completos (o levantamento não lhes toca) e %ld"
               " cresceram\n", ident, cresceu);
        if(ident == 0 || cresceu == 0) mal++;
        printf("\n");
        ok("O LEVANTAMENTO COMPLETA E NÃO INVENTA. Nos quatro casos π∘ι devolve os mesmos"
           " objectos com as mesmas multiplicidades, e lá em cima G é constante por"
           " construção. Quem já era quociente NÃO CRESCE --- o levantamento é a identidade"
           " nele, e é isso que o distingue de um enchimento qualquer; quem resume ganha"
           " exactamente os lugares que a falta contava, nem mais um. E os objectos de"
           " partida continuam todos vivos lá em cima: o que se abre são lugares, não"
           " objectos --- dá-se sítio a quem já disputava o mesmo endereço.", mal == 0);
    }

    /* ── §L2 O GUME: TIRAR UMA FOLHA E A COMPLETUDE CAI ─────────────────── */
    {
        printf("§L2  o gume, uma folha de cada vez.\n\n");
        long mal = 0;
        long end[36]; long n = 0;
        for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++) end[n++] = (a + b) % 6;
        LvLevanta L;
        if(!lv_levanta(end, n, &L)) mal++;
        printf("      o levantado: %ld objectos, G = %ld, falta %ld\n",
               L.n, L.gmax, es_falta(L.base, L.n));
        /* tirar UMA folha, em CADA posição possível: em todas a completude cai */
        long caiu = 0, testadas = 0;
        for(long k = 0; k < L.n; k++){
            long alt[LV_MAX]; long m = 0;
            for(long j = 0; j < L.n; j++) if(j != k) alt[m++] = L.base[j];
            EsFibra f = es_fibra(alt, m);
            testadas++;
            if(!f.constante && es_falta(alt, m) > 0) caiu++;
        }
        printf("      tirada UMA folha em cada uma das %ld posições: a completude cai"
               " em %ld\n", testadas, caiu);
        if(caiu != testadas) mal++;
        /* e acrescentar uma folha a mais TAMBÉM parte a constância */
        {
            long alt[LV_MAX]; long m = 0;
            for(long j = 0; j < L.n; j++) alt[m++] = L.base[j];
            alt[m++] = L.base[0];
            EsFibra f = es_fibra(alt, m);
            printf("      acrescentada UMA a mais: G passa a %ld..%ld, falta %ld\n",
                   f.menor, f.maior, es_falta(alt, m));
            if(f.constante) mal++;
        }
        printf("\n");
        ok("A COMPLETUDE É FRÁGIL NOS DOIS SENTIDOS, E TEM DE SER. Tirada uma folha em"
           " QUALQUER das posições --- não numa escolhida a jeito --- G deixa de ser"
           " constante e a falta reaparece, em todas elas. E acrescentar uma a mais parte a"
           " constância do mesmo modo: o alvo sobe e todas as outras fibras passam a dever"
           " uma. É o gume que faltava ao `es_falta`: um medidor que só confirmasse o caso"
           " bom não distinguia «G constante» de «contei mal».", mal == 0);
    }

    /* ── §L3 ISOMORFIA: a fibra não vê mais nada ────────────────────────── */
    {
        printf("§L3  dois corpos completos com a mesma assinatura são o mesmo corpo.\n\n");
        long mal = 0;
        LvLevanta A, B, C;
        { long e[36]; long n = 0;
          for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++) e[n++] = a + b;
          if(!lv_levanta(e, n, &A)) mal++; }
        { long e[36]; long n = 0;   /* leitura DIFERENTE, mesma forma de fibra */
          for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++) e[n++] = 10*(a+b) + 3;
          if(!lv_levanta(e, n, &B)) mal++; }
        { long e[36]; long n = 0;   /* e uma que resume de outra maneira */
          for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++) e[n++] = a*a + b*b;
          if(!lv_levanta(e, n, &C)) mal++; }
        printf("      A: n=%ld G=%ld fibras=%ld\n", A.n, A.gmax, A.fibras);
        printf("      B: n=%ld G=%ld fibras=%ld   --- A≅B: %s\n", B.n, B.gmax, B.fibras,
               lv_isomorfo(&A, &B) ? "sim" : "nao");
        printf("      C: n=%ld G=%ld fibras=%ld   --- A≅C: %s\n", C.n, C.gmax, C.fibras,
               lv_isomorfo(&A, &C) ? "sim" : "nao");
        if(!lv_isomorfo(&A, &B)) mal++;
        if(lv_isomorfo(&A, &C)) mal++;      /* e este é o gume: NEM TODOS são */
        printf("\n");
        ok("A ASSINATURA DECIDE, E NÃO DECIDE TUDO. Duas leituras diferentes --- a soma e a"
           " soma esticada por dez e deslocada por três --- dão levantados com a mesma"
           " assinatura, e são o mesmo corpo: a fibra não vê o nome dos endereços, só quantos"
           " são e de que tamanho. MAS A SOMA DE QUADRADOS NÃO É ISOMORFA A NENHUMA DELAS, e"
           " essa é a metade que interessa: se o critério aceitasse tudo não estaria a"
           " decidir nada. É o mesmo gume de sempre --- uma condição que nunca falha não é"
           " uma condição.", mal == 0);
    }

    return falhas ? 1 : 0;
}
