/* tests/exaustao.c — A EXAUSTÃO EM OITO BITS: 128 pontos, e varre-se TODOS.
 *
 * O eval pôs a exaustão antes do tipo — «aí o int8_t deixa de ser uma aposta e vira
 * consequência» — e deu a regra que a torna possível:
 *
 *      «Não representar o infinito como um número grande.
 *       Representá-lo como o DUAL PROJECTIVO DE ZERO.»
 *
 * E há o encaixe: 127 é PRIMO e é o topo do `int8_t`, logo ℙ¹(𝔽₁₂₇) tem 128 pontos —
 * exactamente os valores não negativos do tipo. Num corpo finito nada cresce, e o espaço
 * é pequeno o bastante para se varrer INTEIRO: não há amostra, não há tecto, não há
 * profundidade escolhida. É a única varredura desta casa que não pode estar a deixar de
 * fora o caso interessante, porque não há caso de fora.
 *
 * §X0  os 128 pontos existem e são distintos — e o ∞ é [1:0], não um número grande
 * §X1  a INVERSÃO é total e involutiva nos 128, e 0 ↔ ∞ — e ν = −1/x também
 * §X2  o GATO é bijecção dos 128 para todo metal — os 126 metais, exaustivo
 * §X3  a órbita do gato FECHA sempre, e o período mede-se (nada cresce)
 * §X4  a Möbius com det ≠ 0 é bijecção — exaustivo nos pontos, amostrado nas matrizes
 * §X5  o gume: com det = 0 ela COLAPSA, e mede-se quantos pontos sobram
 * §X6  e o que NÃO cabe: 𝔽₁₂₇ não é ℚ — o que aqui é exaustivo, lá é uma face
 */
#include <stdio.h>
#include "oito.h"
#include "unidade.h"

int main(void){
    printf("\n=== A EXAUSTÃO EM OITO BITS: ℙ¹(𝔽₁₂₇), 128 pontos ===\n");

    /* ═══ §X0 OS 128 PONTOS ══════════════════════════════════════════════════ */
    printf("\n§X0 Os 128 pontos, e o infinito é [1:0].\n\n");
    {
        int visto[OT_PONTOS];
        for(int i = 0; i < OT_PONTOS; i++) visto[i] = 0;
        long n = 0, dup = 0;
        for(int q = 0; q < OT_P; q++) for(int p = 0; p < OT_P; p++){
            Pt x;
            if(!ot_ponto(p, q, &x)) continue;
            if(!visto[(int)x]){ visto[(int)x] = 1; n++; } else dup++;
        }
        long faltam = 0;
        for(int i = 0; i < OT_PONTOS; i++) if(!visto[i]) faltam++;
        printf("      pontos distintos alcançados: %ld de %d;  faltam %ld\n",
               n, OT_PONTOS, faltam);
        printf("      e o ∞ é o índice %d — UM int8_t, não um número grande\n",
               (int)OT_INF);
        ok("OS 128 PONTOS DE ℙ¹(𝔽₁₂₇) EXISTEM TODOS E CABEM NUM `int8_t`, e o infinito é"
           " um deles — o ponto [1:0], guardado como o índice 127. É a regra que o eval"
           " deu: não representar o infinito como um número grande, mas como o dual"
           " projectivo do zero. O encaixe não foi procurado: 127 é primo E é o topo do"
           " tipo, logo o corpo cabe inteiro e a recta projectiva tem exactamente 128"
           " pontos — os valores não negativos do `int8_t`",
           n == OT_PONTOS && faltam == 0 && ot_fora == 0);
    }

    /* ═══ §X1 A INVERSÃO, EXAUSTIVA NOS 128 ═════════════════════════════════ */
    printf("\n§X1 A inversão é total e involutiva — nos 128, sem excepção.\n\n");
    {
        long mal = 0, n = 0;
        int img[OT_PONTOS];
        for(int i = 0; i < OT_PONTOS; i++) img[i] = 0;
        for(int i = 0; i < OT_PONTOS; i++){
            Pt x = (Pt)i, y = ot_inverte(x), z = ot_inverte(y);
            n++;
            if(z != x) mal++;                       /* involução */
            img[(int)y]++;
        }
        long colide = 0;
        for(int i = 0; i < OT_PONTOS; i++) if(img[i] != 1) colide++;
        int zi = (ot_inverte((Pt)0) == OT_INF), iz = (ot_inverte(OT_INF) == (Pt)0);
        printf("      ι∘ι = id em %ld/%d pontos: %ld divergências;  não é bijecção em"
               " %ld\n", n, OT_PONTOS, mal, colide);
        printf("      1/0 = ∞ ? %s      1/∞ = 0 ? %s\n", zi ? "sim" : "NÃO",
               iz ? "sim" : "NÃO");
        /* E A OUTRA INVOLUÇÃO DA CASA, ν(x) = −1/x, que é a Lei 1 — e que vivia em
         * `lib/oito.h` SEM UM ÚNICO CHAMADOR, logo sem nunca ter sido medida. Ao pô-la a
         * correr nos 128 apareceu que não era involução: o oposto de ∞ estava escrito
         * como zero, o que dava ν(∞) = ∞ e portanto ν(ν(0)) = ∞ ≠ 0 — falhava exactamente
         * no par que a Lei 0 nomeia. Com o oposto certo (−∞ = ∞) fecha nos 128.
         *
         * E ν não tem ponto fixo nenhum, o que também é lei e não acaso: ν(x) = x pede
         * x² = −1, e −1 não é resíduo quadrático módulo 127 (porque 127 ≡ 3 mod 4). O i
         * não vive nesta face — e é por isso que aqui ν roda sem nada ficar parado. */
        long nu_inv = 0, nu_fix = 0, nu_orb = 0;
        for(int i = 0; i < OT_PONTOS; i++){
            Pt x = (Pt)i, y = ot_nu(x);
            if(ot_nu(y) == x) nu_inv++;
            if(y == x) nu_fix++;
            if((x == 0 && y == OT_INF) || (x == OT_INF && y == 0)) nu_orb++;
        }
        printf("      ν∘ν = id em %ld/%d · pontos fixos de ν: %ld (x² = −1 não tem solução"
               " em 𝔽₁₂₇) · a órbita 0 ↔ ∞: %ld\n", nu_inv, OT_PONTOS, nu_fix, nu_orb);
        ok("E ν(x) = −1/x É INVOLUÇÃO NOS 128, COM O PAR 0 ↔ ∞ COMO ÓRBITA: esta função"
           " vivia na biblioteca sem um único chamador, logo nunca tinha sido medida — e"
           " ao correr apareceu que falhava, porque o oposto de ∞ estava escrito como"
           " zero. Falhava exactamente no par que a Lei 0 nomeia, que é onde só a"
           " exaustão olha. E ν não tem ponto fixo, o que é lei: ν(x) = x pede x² = −1, e"
           " −1 não é resíduo quadrático módulo 127",
           nu_inv == OT_PONTOS && nu_fix == 0 && nu_orb == 2);
        ok("A INVERSÃO É TOTAL, INVOLUTIVA E BIJECTIVA NOS 128 PONTOS — e isto é EXAUSTIVO,"
           " não uma amostra: são todos os pontos que existem. O zero tem inversa (o ∞) e"
           " o ∞ tem inversa (o zero), e nenhum ponto colide com outro. A fibra vazia que"
           " esta casa escrevia em todo andar não tem aqui um único sítio onde aparecer",
           mal == 0 && colide == 0 && zi && iz && n == OT_PONTOS);
    }

    /* ═══ §X2 O GATO É BIJECÇÃO — EXAUSTIVO NOS METAIS E NOS PONTOS ═════════ */
    printf("\n§X2 O gato é bijecção dos 128, para TODO metal.\n\n");
    {
        long metais = 0, mal = 0;
        for(int m = 1; m < OT_P; m++){
            int img[OT_PONTOS];
            for(int i = 0; i < OT_PONTOS; i++) img[i] = 0;
            int falhou = 0;
            for(int i = 0; i < OT_PONTOS; i++){
                Pt y;
                if(!ot_gato((F)m, (Pt)i, &y)){ falhou = 1; break; }
                img[(int)y]++;
            }
            metais++;
            if(falhou){ mal++; continue; }
            for(int i = 0; i < OT_PONTOS; i++) if(img[i] != 1){ mal++; break; }
        }
        /* e o 0 ↦ ∞ ↦ m, exaustivo */
        long trav = 0;
        for(int m = 1; m < OT_P; m++){
            Pt a, b;
            if(ot_gato((F)m, (Pt)0, &a) && ot_e_inf(a)
               && ot_gato((F)m, a, &b) && b == (Pt)m) trav++;
        }
        printf("      bijecção em %ld/%ld metais: %ld falhas\n", metais - mal, metais, mal);
        printf("      e 0 ↦ ∞ ↦ m em %ld dos %ld — o gato ATRAVESSA o infinito, sempre\n",
               trav, metais);
        ok("O GATO É BIJECÇÃO DOS 128 PONTOS PARA TODO METAL, e isto é exaustivo nas duas"
           " direcções: todos os 126 metais e todos os 128 pontos de cada. E em todos eles"
           " a órbita a partir do zero atravessa o infinito e cai em m — que é o"
           " Corolário 0 ↔ ∞ verificado sem uma única lacuna, no único sítio onde «sem"
           " lacuna» é uma afirmação e não uma esperança",
           mal == 0 && trav == metais && metais == OT_P - 1);
    }

    /* ═══ §X3 A ÓRBITA FECHA SEMPRE, E O PERÍODO MEDE-SE ════════════════════ */
    printf("\n§X3 Num corpo finito nada cresce: a órbita fecha, e conta-se.\n\n");
    {
        long fecha = 0, n = 0, menor = 1 << 30, maior = 0;
        for(int m = 1; m < OT_P; m++){
            long per;
            n++;
            if(ot_periodo_gato((F)m, &per)){
                fecha++;
                if(per < menor) menor = per;
                if(per > maior) maior = per;
            }
        }
        printf("      a órbita fecha em %ld/%ld metais;  período entre %ld e %ld\n",
               fecha, n, menor, maior);
        printf("      e nada cresceu: o maior valor guardado continua a caber num"
               " int8_t\n");
        ok("A ÓRBITA DO GATO FECHA SEMPRE, E O PERÍODO É UM NÚMERO — não uma esperança."
           " Sobre ℤ o gato cresce como σᵏ e satura em toda representação finita; em 𝔽₁₂₇"
           " ele roda, porque o grupo é finito. É o Teorema do Esquilo medido: o RAMO é"
           " propriedade da realização, e a lei não. E aqui a consequência é prática — não"
           " há prova por crescimento possível, porque nada cresce",
           fecha == n && n == OT_P - 1 && maior > 0 && ot_fora == 0);
    }

    /* ═══ §X4 A MÖBIUS COM det ≠ 0 É BIJECÇÃO ══════════════════════════════ */
    printf("\n§X4 Möbius com det ≠ 0: bijecção dos 128, exaustivo nos pontos.\n\n");
    {
        long mats = 0, mal = 0;
        for(int a = 0; a < OT_P; a += 11) for(int b = 0; b < OT_P; b += 13)
        for(int c = 0; c < OT_P; c += 17) for(int d = 0; d < OT_P; d += 19){
            if(ot_menos(ot_mult((F)a,(F)d), ot_mult((F)b,(F)c)) == 0) continue;
            int img[OT_PONTOS];
            for(int i = 0; i < OT_PONTOS; i++) img[i] = 0;
            int falhou = 0;
            for(int i = 0; i < OT_PONTOS; i++){
                Pt y;
                if(!ot_mobius((F)a,(F)b,(F)c,(F)d,(Pt)i,&y)){ falhou = 1; break; }
                img[(int)y]++;
            }
            mats++;
            if(falhou){ mal++; continue; }
            for(int i = 0; i < OT_PONTOS; i++) if(img[i] != 1){ mal++; break; }
        }
        printf("      %ld matrizes com det ≠ 0, cada uma varrida nos 128 pontos: %ld"
               " falhas\n", mats, mal);
        ok("A ACÇÃO DE MÖBIUS COM det ≠ 0 É BIJECÇÃO DOS 128, e nos PONTOS a varredura é"
           " exaustiva — cada matriz é aplicada a todos os pontos que existem. Nas"
           " matrizes é amostrada, e digo-o: 127⁴ são 260 milhões, e varrê-las seria"
           " trocar uma exaustão honesta por uma espera. O que se exaure é o objecto sobre"
           " o qual a lei fala",
           mal == 0 && mats > 100);
    }

    /* ═══ §X5 O GUME: com det = 0 ela COLAPSA ══════════════════════════════ */
    printf("\n§X5 O gume: com det = 0 a recta colapsa, e conta-se em quantos pontos.\n\n");
    {
        long deg = 0, recusou = 0;
        long colapso_min = OT_PONTOS, colapso_max = 0;
        for(int a = 0; a < OT_P; a += 7) for(int b = 0; b < OT_P; b += 7)
        for(int c = 0; c < OT_P; c += 7) for(int d = 0; d < OT_P; d += 7){
            if(ot_menos(ot_mult((F)a,(F)d), ot_mult((F)b,(F)c)) != 0) continue;
            deg++;
            Pt y;
            if(!ot_mobius((F)a,(F)b,(F)c,(F)d,(Pt)3,&y)) recusou++;
            /* e quantos pontos DISTINTOS a imagem teria, se a deixássemos correr */
            int img[OT_PONTOS];
            for(int i = 0; i < OT_PONTOS; i++) img[i] = 0;
            long dist = 0;
            for(int i = 0; i < OT_PONTOS; i++){
                int p = ot_e_inf((Pt)i) ? 1 : i, q = ot_e_inf((Pt)i) ? 0 : 1;
                Pt z;
                if(ot_ponto(a*p + b*q, c*p + d*q, &z) && !img[(int)z]){
                    img[(int)z] = 1; dist++;
                }
            }
            if(dist < colapso_min) colapso_min = dist;
            if(dist > colapso_max) colapso_max = dist;
        }
        printf("      %ld matrizes degeneradas: %ld recusadas pela função\n", deg, recusou);
        printf("      e se as deixássemos correr, a imagem teria entre %ld e %ld pontos"
               " distintos — de 128\n", colapso_min, colapso_max);
        ok("E O GUME MOSTRA O QUE A HIPÓTESE SEGURA: com det = 0 a recta COLAPSA — a"
           " imagem cai a um punhado de pontos em vez dos 128 —, e a função recusa em"
           " todas. O número do colapso é a medida do que se perde, e é ele que faz da"
           " condição det ≠ 0 uma hipótese que trabalha em vez de uma cerimónia",
           recusou == deg && deg > 0 && colapso_max < OT_PONTOS);
    }

    /* ═══ §X6 E O QUE ISTO NÃO É ═══════════════════════════════════════════ */
    printf("\n§X6 O que a exaustão NÃO prova.\n\n");
    {
        /* 𝔽₁₂₇ tem características que ℚ não tem: o 127·x = 0 para todo x */
        long car = 0;
        for(int x = 0; x < OT_P; x++){
            int s = 0;
            for(int k = 0; k < OT_P; k++) s = ot_soma((F)s, (F)x);
            if(s == 0) car++;
        }
        printf("      somar x consigo próprio 127 vezes dá zero em %ld/%d elementos —"
               " a característica\n", car, OT_P);
        printf("      e em ℚ isso NUNCA acontece: é uma propriedade da face, não da lei\n");
        ok("E O QUE A EXAUSTÃO NÃO PROVA DIZ-SE, senão ela valia menos do que parece: 𝔽₁₂₇"
           " tem CARACTERÍSTICA 127 — somar um elemento consigo próprio 127 vezes dá zero,"
           " em todos eles — e em ℚ isso nunca acontece. Logo o que aqui se varre por"
           " inteiro é uma FACE, e o Teorema do Gato já diz o que isso vale: a lei é"
           " universal, a face é da instância. O que 𝔽₁₂₇ dá é uma face onde não há nada"
           " por varrer — e por isso nenhuma afirmação pode estar a esconder o caso que"
           " faltava",
           car == OT_P);
    }

    printf("\n=== %d asserções, %d falhas, %ld fora do tipo ===\n",
           unidades, falhas, ot_fora);
    return falhas ? 1 : 0;
}
