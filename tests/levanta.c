/* levanta.c — COMPLETAR um corpo pelo ι/π, medido como o cliente usa.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/levanta tests/levanta.c && /tmp/levanta
 */
#include "unidade.h"
#include "levanta.h"
#include "triade.h"
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

    /* ── §L3 A BIJEÇÃO ENTRE AS DUAS RÉGUAS, EXIBIDA ────────────────────── */
    {
        printf("§L3  a travessia entre duas leituras do corpo completado: a função exacta.\n\n");
        long mal = 0;

        /* O corpo levantado tem 66 objectos; a régua que os endereça é a de
         * M = 4, n = 3 (64 endereços) e mais os que sobram --- e para a
         * travessia o que conta é a GRADE, que a `prop:travessia` fixa como
         * I_M^n. Toma-se M = 4, n = 3: N = 64, que é onde o corpo vive. */
        long M = 4; int n = 3, N = 64;
        int r[3] = {0, 1, 2};          /* a leitura de partida: a identidade */
        int s[3] = {2, 0, 1};          /* a outra: roda as posições */

        /* (1) T = S∘R⁻¹ É BIJEÇÃO --- e não se procura: lê-se o dígito de uma
         * posição e escreve-se noutra (prop:travessia (1)). */
        long visto[64]; long fecha = 0;
        for(long a = 0; a < N; a++) visto[a] = 0;
        for(long a = 0; a < N; a++){
            long t = tv_travessia(a, M, n, r, s);
            if(t >= 0 && t < N) visto[t]++;
        }
        for(long a = 0; a < N; a++) if(visto[a] == 1) fecha++;
        printf("      T = S∘R⁻¹ sobre %d endereços: %ld imagens únicas --- %s\n",
               N, fecha, fecha == N ? "é bijeção" : "NÃO fecha");
        if(fecha != N) mal++;

        /* (2) CONSERVA A MEDIDA: μ(T(E)) = μ(E), e a medida é a contagem. */
        long medida_ok = 0, testes = 0;
        for(long corte = 1; corte <= 8; corte++){
            long img[64]; long m = 0;
            for(long a = 0; a < N; a++) if(a % corte == 0){
                long t = tv_travessia(a, M, n, r, s);
                int novo_ = 1;
                for(long j2 = 0; j2 < m; j2++) if(img[j2] == t){ novo_ = 0; break; }
                if(novo_) img[m++] = t;
            }
            long orig = 0;
            for(long a = 0; a < N; a++) if(a % corte == 0) orig++;
            testes++; if(m == orig) medida_ok++;
        }
        printf("      μ(T(E)) = μ(E) em %ld de %ld cortes --- a medida é a contagem\n",
               medida_ok, testes);
        if(medida_ok != testes) mal++;

        /* (3) E O PREÇO LÊ-SE DA RÉGUA, sem correr nada: D(R,S) = 2^{−q}, com q
         * a primeira posição que π move contada do dígito mais significativo. E
         * o gume é por LEI: não uma permutação escolhida a jeito --- TODAS as
         * seis ---, com a fórmula a ter de bater com a ultramétrica em cada uma
         * E a dar preços DIFERENTES entre elas. Um q constante passaria por
         * acaso em qualquer fórmula. */
        {
            int P[6][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
            long batem = 0; int vistos[8] = {0}; long distintos = 0;
            printf("      π          q_dig  q_bit  a régua mede  bate\n");
            for(int t2 = 0; t2 < 6; t2++){
                int *sp = P[t2];
                int qd = tv_preco(n, r, sp), qbb = tv_preco_bits(n, r, sp, 2);
                int pior2 = 64, moveu = 0;
                for(long a = 0; a < N; a++){
                    long im = tv_travessia(a, M, n, r, sp);
                    if(im == a) continue;
                    moveu = 1;
                    int pp = tv_prof(a, im, 6);
                    if(pp < pior2) pior2 = pp;
                }
                if(!moveu) pior2 = qbb;         /* π = id: nada se move */
                printf("      (%d,%d,%d) %6d %6d %12d  %s\n", sp[0], sp[1], sp[2],
                       qd, qbb, pior2, pior2 == qbb ? "sim" : "NAO");
                if(pior2 == qbb) batem++;
                if(qbb < 8 && !vistos[qbb]){ vistos[qbb] = 1; distintos++; }
            }
            printf("      → %ld de 6 batem, e saem %ld preços DISTINTOS --- se fosse"
                   " sempre o mesmo, a fórmula passava por acaso\n", batem, distintos);
            if(batem != 6 || distintos < 3) mal++;
        }

        /* E A IDENTIDADE NÃO TEM PREÇO: π = id devolve n, e a travessia é a
         * identidade --- a metade que tem de existir para o preço significar algo. */
        int s_id[3] = {0, 1, 2};
        long fixos = 0;
        for(long a = 0; a < N; a++) if(tv_travessia(a, M, n, r, s_id) == a) fixos++;
        printf("      π = id: %ld dos %d endereços fixos, e o preço devolve %d (= n)\n",
               fixos, N, tv_preco(n, r, s_id));
        if(fixos != N || tv_preco(n, r, s_id) != n) mal++;

        /* ── E A ASSINATURA DE FIBRA NÃO É UM CRITÉRIO DE ISOMORFIA. ────────
         * Ela compara três números; a isomorfia tem a travessia acima, que é
         * uma FUNÇÃO exibida. Guardar as duas coisas com o mesmo nome era o
         * erro: negar isomorfia por assinatura contradiz o `cor:global`. */
        LvLevanta A, B;
        { long e[36]; long m = 0;
          for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++) e[m++] = a + b;
          if(!lv_levanta(e, m, &A)) mal++; }
        { long e[36]; long m = 0;
          for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++) e[m++] = a*a + b*b;
          if(!lv_levanta(e, m, &B)) mal++; }
        printf("      soma: n=%ld G=%ld · quadrados: n=%ld G=%ld · mesma fibra: %s\n",
               A.n, A.gmax, B.n, B.gmax, lv_mesma_fibra(&A, &B) ? "sim" : "nao");
        long viva = lv_travessia_viva(&A, &B, 36);
        printf("      e os objectos VIVOS emparelham: %ld de 36 --- a travessia existe\n",
               viva);
        if(viva != 36) mal++;

        printf("\n");
        ok("A BIJEÇÃO ENTRE AS DUAS RÉGUAS NÃO SE ARGUMENTA: ESCREVE-SE. É T = S∘R⁻¹ da"
           " prop:travessia, a permutação π = s∘r⁻¹ aplicada às POSIÇÕES dos dígitos --- não"
           " há nada a procurar, lê-se o dígito de uma posição e escreve-se noutra. Fecha nos"
           " 64 endereços, conserva a medida em todos os cortes, e o preço lê-se da régua sem"
           " correr nada: a fórmula dá q e a ultramétrica MEDE o mesmo q. A identidade não"
           " tem preço, que é a metade sem a qual o preço não significaria nada. E o que a"
           " assinatura de fibra compara são três números: NÃO é um critério de isomorfia, e"
           " tratá-la como tal contradizia o cor:global, que dá travessia entre quaisquer"
           " duas codificações reversíveis. Os objectos vivos emparelham nos dois"
           " levantamentos: a travessia existe onde o corolário a promete.", mal == 0);
    }

    return falhas ? 1 : 0;
}
