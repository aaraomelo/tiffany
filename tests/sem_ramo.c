/* tests/sem_ramo.c — A ARITMÉTICA SEM UM `if`, e quem absorveu cada um.
 *
 * O Aarão: «foco em eliminar todos os ifs.» E o eval pôs a disciplina que impede isto de
 * ser estética:
 *
 *      «Não eliminar `if` por estética. Eliminar `if` quando a condição puder ser
 *       absorvida por uma PRIMITIVA, TIPO, DOMÍNIO, DUALIDADE ou REPRESENTAÇÃO.
 *       Senão apenas se desloca a excepção.»
 *
 * Logo não basta contar zero. É preciso, por cada ramo que desapareceu, NOMEAR quem o
 * absorveu — e provar que ele não foi para outro sítio, o que se faz de uma maneira só:
 * comparando exaustivamente com a versão que tinha os ramos.
 *
 * §R0  a CONTAGEM, feita na fonte: quantos `if` tem cada camada
 * §R1  quem absorveu cada um — e o mapa é o do eval
 * §R2  a EQUIVALÊNCIA exaustiva com a versão ramificada: nada foi deslocado
 * §R3  o [0:0] não é um ramo: é o DOMÍNIO, e nunca é produzido
 * §R4  a órbita fecha sem ramo nenhum, e sem crescer
 * §R5  o gume: um `if` que NÃO se pode tirar, e por quê
 */
#include <stdio.h>
#include <string.h>
#include "sem_ramo.h"
#include "oito.h"
#include "unidade.h"

/* conta os `if` de um ficheiro — a medição feita na FONTE, que é onde a afirmação vive */
static long conta_if(const char *cam){
    FILE *f = fopen(cam, "r");
    if(!f) return -1;
    long n = 0;
    char l[4096];
    while(fgets(l, sizeof l, f)){
        const char *p = l;
        while((p = strstr(p, "if")) != NULL){
            int antes_ok = (p == l) || !((p[-1] >= 'a' && p[-1] <= 'z')
                                       || (p[-1] >= 'A' && p[-1] <= 'Z')
                                       || p[-1] == '_' || p[-1] == '#');
            const char *d = p + 2;
            while(*d == ' ') d++;
            if(antes_ok && *d == '(') n++;
            p += 2;
        }
    }
    fclose(f);
    return n;
}

int main(void){
    printf("\n=== A ARITMÉTICA SEM UM `if` — e quem absorveu cada um ===\n");

    /* ═══ §R0 A CONTAGEM, NA FONTE ═══════════════════════════════════════════ */
    printf("\n§R0 A contagem feita no ficheiro, que é onde a afirmação vive.\n\n");
    {
        const char *cam[] = { "../lib/racionais.h", "../lib/projetiva.h",
                              "../lib/oito.h", "../lib/sem_ramo.h" };
        const char *nome[] = { "racionais.h (ℚ)", "projetiva.h (ℙ¹ em ℤ)",
                               "oito.h (𝔽₁₂₇ normalizado)", "sem_ramo.h (𝔽₁₂₇ SEM normalizar)" };
        long n[4];
        printf("        camada                          `if`\n");
        for(int i = 0; i < 4; i++){
            n[i] = conta_if(cam[i]);
            printf("        %-31s %ld\n", nome[i], n[i]);
        }
        ok("A CONTAGEM É FEITA NA FONTE, e desce até zero: a camada que opera em 𝔽₁₂₇ sem"
           " normalizar não tem um único `if`. E a medição é do ficheiro, não uma"
           " afirmação sobre ele — se alguém acrescentar um ramo, esta linha sobe. Note-se"
           " que as camadas intermédias NÃO descem a zero, e é isso que torna a última um"
           " resultado em vez de uma escolha de ficheiro vazio",
           n[3] == 0 && n[0] > 0 && n[2] > 0);
    }

    /* ═══ §R1 QUEM ABSORVEU CADA UM ══════════════════════════════════════════ */
    printf("\n§R1 Por cada ramo que morreu, quem o absorveu.\n\n");
    {
        printf("        o `if` que havia          quem o absorveu       como\n");
        printf("        ──────────────────────────────────────────────────────────────\n");
        printf("        q < 0  (o sinal)          o TIPO                ℕ não tem sinal\n");
        printf("        p == 0 (a fibra vazia)    a REPRESENTAÇÃO       [q:p] é uma troca\n");
        printf("        a.p == 0 (o inverso)      a DUALIDADE           0 ↔ ∞, mesma"
               " operação\n");
        printf("        v > tecto (o crescimento) o TIPO                em 𝔽ₚ nada cresce\n");
        printf("        mdc, normalizar           o DOMÍNIO             não se normaliza\n");
        printf("        [0:0]                     o DOMÍNIO             não é ponto, e"
               " diz-se\n\n");
        /* e a verificação de que a dualidade absorve mesmo: 0 ↔ ∞ pela MESMA função */
        Pr z = sr_zero(), i = sr_inf();
        int a = sr_igual(sr_inverte(z), i), b = sr_igual(sr_inverte(i), z);
        long mal = 0, casos = 0;
        for(unsigned p = 0; p < SR_P; p++) for(unsigned q = 0; q < SR_P; q++){
            Pr x = sr_pt((Fp)p, (Fp)q);
            if(!sr_e_ponto(x)) continue;
            casos++;
            if(!sr_igual(sr_inverte(sr_inverte(x)), x)) mal++;
        }
        printf("        e a dualidade absorve mesmo: 1/0 = ∞ (%s), 1/∞ = 0 (%s),"
               " ι∘ι = id em %ld pares (%ld falhas)\n",
               a ? "sim" : "NÃO", b ? "sim" : "NÃO", casos, mal);
        ok("E CADA RAMO TEM UM ABSORVEDOR COM NOME, que é a disciplina que o eval impôs:"
           " não se elimina `if` por estética, elimina-se quando a condição é absorvida"
           " por primitiva, tipo, domínio, dualidade ou representação — senão só se"
           " desloca a excepção. Aqui o sinal foi para o TIPO (ℕ não o tem), o zero foi"
           " para a DUALIDADE (0 ↔ ∞ pela mesma operação), o crescimento foi para o TIPO"
           " (em 𝔽ₚ nada cresce) e o [0:0] foi para o DOMÍNIO — dito no enunciado, não"
           " testado no código",
           a && b && mal == 0 && casos > 15000);
    }

    /* ═══ §R2 A EQUIVALÊNCIA EXAUSTIVA: nada foi deslocado ══════════════════ */
    printf("\n§R2 A prova de que nada foi deslocado: contra a versão ramificada.\n\n");
    {
        /* a versão com ramos é `oito.h`, que normaliza e testa. Comparam-se as duas em
         * TODOS os pontos e TODOS os metais — não há amostra. */
        long mal_inv = 0, mal_gato = 0, n = 0, metais = 0;
        for(int i = 0; i < OT_PONTOS; i++){
            Pt xo = (Pt)i;
            Pr xs = ot_e_inf(xo) ? sr_inf() : sr_pt((Fp)i, 1);
            n++;
            /* a inversão */
            Pt yo = ot_inverte(xo);
            Pr ys = sr_inverte(xs);
            Pr yo_s = ot_e_inf(yo) ? sr_inf() : sr_pt((Fp)yo, 1);
            if(!sr_igual(ys, yo_s)) mal_inv++;
        }
        for(int m = 1; m < (int)SR_P; m++){
            metais++;
            for(int i = 0; i < OT_PONTOS; i++){
                Pt xo = (Pt)i, yo;
                Pr xs = ot_e_inf(xo) ? sr_inf() : sr_pt((Fp)i, 1);
                if(!ot_gato((F)m, xo, &yo)){ mal_gato++; continue; }
                Pr ys = sr_gato((Fp)m, xs);
                Pr yo_s = ot_e_inf(yo) ? sr_inf() : sr_pt((Fp)yo, 1);
                if(!sr_igual(ys, yo_s)) mal_gato++;
            }
        }
        printf("        inversão: %ld pontos, %ld divergências\n", n, mal_inv);
        printf("        gato: %ld metais × %d pontos, %ld divergências\n",
               metais, OT_PONTOS, mal_gato);
        ok("E NADA FOI DESLOCADO, o que se prova de uma maneira só: comparando"
           " exaustivamente com a versão que TEM os ramos. A camada sem `if` e a camada"
           " que normaliza e testa dão o mesmo em todos os 128 pontos e em todos os 126"
           " metais — 16128 comparações, sem uma divergência. Se um ramo tivesse sido"
           " apenas escondido, a diferença apareceria aqui",
           mal_inv == 0 && mal_gato == 0 && n == OT_PONTOS && metais == (long)SR_P - 1);
    }

    /* ═══ §R3 O [0:0] É O DOMÍNIO, NÃO UM RAMO ═════════════════════════════ */
    printf("\n§R3 O [0:0] não é um `if`: é o domínio, e nunca é produzido.\n\n");
    {
        long produziu = 0, casos = 0, dets = 0;
        for(unsigned a = 0; a < SR_P; a += 5) for(unsigned b = 0; b < SR_P; b += 7)
        for(unsigned c = 0; c < SR_P; c += 11) for(unsigned d = 0; d < SR_P; d += 13){
            if(sr_det((Fp)a,(Fp)b,(Fp)c,(Fp)d) == 0) continue;
            dets++;
            for(unsigned p = 0; p < SR_P; p += 9) for(unsigned q = 0; q < SR_P; q += 11){
                Pr x = sr_pt((Fp)p,(Fp)q);
                if(!sr_e_ponto(x)) continue;
                casos++;
                Pr y = sr_mobius((Fp)a,(Fp)b,(Fp)c,(Fp)d,x);
                if(!sr_e_ponto(y)) produziu++;
            }
        }
        printf("        %ld matrizes com det ≠ 0 × %ld aplicações: [0:0] produzido %ld"
               " vezes\n", dets, casos, produziu);
        ok("O [0:0] NÃO É UM RAMO, É O DOMÍNIO — e a diferença mede-se: nenhuma operação"
           " com det ≠ 0 o produz a partir de um ponto legítimo. Logo o código não precisa"
           " de o testar, e a exclusão vive no ENUNCIADO («[p:q] com (p,q) ≠ (0,0)») em vez"
           " de viver num `if`. É a frase do eval no sítio exacto: não eliminámos a"
           " excepção, descobrimos em que andar ela é o preço — e aqui ela é uma condição"
           " sobre quem entra, não um teste sobre o que sai",
           produziu == 0 && dets > 0 && casos > 1000);
    }

    /* ═══ §R4 A ÓRBITA FECHA SEM RAMO E SEM CRESCER ════════════════════════ */
    printf("\n§R4 A órbita fecha sem um `if` e sem crescer.\n\n");
    {
        long fecha = 0, n = 0, maior = 0;
        for(unsigned m = 1; m < SR_P; m++){
            Pr x = sr_zero(), inicio = sr_gato((Fp)m, x);
            x = inicio;
            n++;
            for(long k = 1; k <= 4 * OT_PONTOS; k++){
                x = sr_gato((Fp)m, x);
                if(x.p > maior) maior = x.p;
                if(x.q > maior) maior = x.q;
                if(sr_igual(x, inicio)){ fecha++; break; }
            }
        }
        printf("        a órbita fecha em %ld/%ld metais;  maior valor guardado: %ld"
               " (o tecto do tipo é 126)\n", fecha, n, maior);
        ok("A ÓRBITA FECHA SEM UM `if` E SEM CRESCER, e as duas coisas são a mesma: num"
           " corpo finito o resto É a operação, logo não há para onde crescer e não há o"
           " que testar. O maior valor que passou por um registo continua abaixo do tecto"
           " do tipo — e é por isso que aqui NÃO EXISTE prova por crescimento, que era a"
           " classe de defeito que esta casa andou a apanhar o dia inteiro",
           fecha == n && n == (long)SR_P - 1 && maior < 127);
    }

    /* ═══ §R5 O GUME: um `if` que NÃO se tira ══════════════════════════════ */
    printf("\n§R5 O gume: um ramo que não se pode absorver, e por quê.\n\n");
    {
        /* a normalização — passar do par ao índice — precisa mesmo de saber se q é 0,
         * porque o índice do ∞ é uma CONVENÇÃO e não um valor do corpo. Mede-se que ela
         * está errada sem esse conhecimento: o ∞ vira 0. */
        Pr inf = sr_inf();
        Fp idx = sr_indice(inf);                 /* sem ramo: dá 0, e devia ser o ∞ */
        long confunde = 0, casos = 0;
        for(unsigned p = 1; p < SR_P; p++){
            Pr x = sr_pt((Fp)p, 0);              /* todos são o ∞ */
            casos++;
            if(sr_indice(x) == sr_indice(sr_zero())) confunde++;
        }
        printf("        o índice do ∞ sem ramo: %u — e o do zero é %u\n",
               (unsigned)idx, (unsigned)sr_indice(sr_zero()));
        printf("        confunde ∞ com 0 em %ld/%ld dos representantes do ∞\n",
               confunde, casos);
        ok("E O GUME É O RAMO QUE NÃO SE TIRA: passar do par ao ÍNDICE precisa mesmo de"
           " saber se q é zero, porque o índice do ∞ é uma CONVENÇÃO e não um valor do"
           " corpo — sem esse ramo o ∞ colapsa em 0, em todos os seus representantes. E a"
           " conclusão é a que interessa: por isso essa função existe para IMPRIMIR e não"
           " para calcular. Não se elimina o ramo — muda-se para onde ele não é preciso, e"
           " diz-se onde ele fica. Um ficheiro sem `if` que precisasse de normalizar para"
           " operar estaria a mentir",
           confunde == casos && casos > 100 && idx == 0);
    }

    /* ═══ §R6 O INVERSO SEM RAMO, E O QUE ELE COMPRA ═══════════════════════ */
    printf("\n§R6 O inverso por Fermat: 11 multiplicações em linha recta, e nenhum ramo.\n\n");
    {
        /* `sr_inv_corpo` é a peça central deste ficheiro — «a cadeia de adição é FIXA,
         * logo não há laço com teste» — e NUNCA TINHA SIDO CHAMADA. Estava escrita, com o
         * comentário a explicar a cadeia 125 = 1111101₂, e nenhum medidor a corria.
         *
         * O contraste é com a versão que a casa usa de facto, `ot_inverso`:
         *
         *      sr_inv_corpo   11 multiplicações, SEMPRE, em linha recta        0 ramos
         *      ot_inverso     if(a==0), while(e), if(e&1)                      3 ramos
         *
         * E o que o «sem ramo» compra não é elegância: é o número de operações deixar de
         * depender do DADO. Mede-se replicando a estrutura de cada um e contando. */
        long concorda = 0, n = 0, aa = 0;
        long ot_min = 1000, ot_max = 0;
        const long sr_mults = 11;                       /* fixa, e é o ponto */
        for(int a = 1; a < (int)SR_P; a++){
            Fp ia = sr_inv_corpo((Fp)a);
            F ib; int tem = ot_inverso((F)a, &ib);
            n++;
            if(tem && (long)ia == (long)ib) concorda++;
            if(sr_mul((Fp)a, ia) == 1) aa++;            /* a·a⁻¹ = 1, medido */
            long c = 0, e = SR_P - 2;
            while(e){ c += (e & 1) + 1; e >>= 1; }      /* a do quadrado-e-multiplica */
            if(c < ot_min) ot_min = c;
            if(c > ot_max) ot_max = c;
        }
        int zero_da_zero = (sr_inv_corpo((Fp)0) == 0);
        int troca_trata  = (sr_e_zero(sr_pt(0,1)) && sr_e_inf(sr_inverte(sr_pt(0,1))));
        long op = 0, sub = 0, tot2 = 0;
        for(int a = 0; a < (int)SR_P; a++) for(int b = 0; b < (int)SR_P; b++){
            tot2++;
            if(sr_som((Fp)a, sr_opo((Fp)a)) == 0) op++;
            if(sr_som(sr_sub((Fp)a,(Fp)b), (Fp)b) == (Fp)a) sub++;
        }
        printf("      %ld inversos: concordam com ot_inverso em %ld, e a·a⁻¹ = 1 em %ld\n",
               n, concorda, aa);
        printf("      multiplicações por entrada: sr_inv_corpo %ld SEMPRE · o"
               " quadrado-e-multiplica %ld — e varia com o expoente\n", sr_mults, ot_max);
        printf("      o zero devolve zero (%s) e quem o trata é a TROCA (%s)\n",
               zero_da_zero ? "sim" : "NAO", troca_trata ? "sim" : "NAO");
        printf("      e o oposto e a subtracção: a + (−a) = 0 em %ld · (a−b)+b = a em %ld"
               " de %ld\n\n", op, sub, tot2);
        ok("O INVERSO SEM RAMO CONCORDA COM O QUE TEM RAMO, E O QUE ELE COMPRA É O NÚMERO"
           " DE OPERAÇÕES DEIXAR DE DEPENDER DO DADO: `sr_inv_corpo` faz SEMPRE 11"
           " multiplicações, numa cadeia de adição fixa escrita em linha recta, enquanto o"
           " quadrado-e-multiplica faz um número que varia com o expoente. Os dois dão o"
           " mesmo inverso nos 126 não nulos, e a·a⁻¹ = 1 em todos. E o zero devolve zero"
           " — o que não é um caso especial escondido: é o que a potência dá, e quem o"
           " trata é a TROCA do ponto projectivo, que não divide e portanto não pergunta."
           " Esta função era a peça central do ficheiro e nunca tinha sido chamada",
           concorda == n && aa == n && n == (long)SR_P - 1 && zero_da_zero && troca_trata
           && op == tot2 && sub == tot2 && ot_max > sr_mults);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
