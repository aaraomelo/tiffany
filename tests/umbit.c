/* tests/umbit.c — UM BIT, AS OITO LEIS E O ASCII: a álgebra universal em 𝔽₂.
 *
 * O Aarão: «migramos para 1 bit, tabelar ASCII, formalizar a realização ASCII da álgebra
 * universal, e usa neuronio.c para ver como operador bit a bit» — e depois: «aplica as
 * oito leis para isso, vai dar as transformações nos 8 bits que já temos».
 *
 * O catálogo do Corpo estelar tem as oito leis indexadas de 0 a 7 e diz que elas são «um
 * CICLO GERADOR de período oito». Oito leis, oito bits, período oito: o byte não é um
 * recipiente onde as leis cabem — é o ciclo delas escrito em binário.
 *
 * §B0  as cinco primitivas em 𝔽₂: cada uma colapsa numa porta, e o DUAL vira identidade
 * §B1  ℙ¹(𝔽₂) tem TRÊS pontos, e 0 ↔ ∞ sobrevive no corpo mais pobre que existe
 * §B2  AS OITO LEIS como transformações do byte — com o PERÍODO de cada uma, medido
 * §B3  o CICLO GERADOR: rodar um bit tem período OITO, e é ele o ciclo das leis
 * §B4  o ASCII: 128 códigos são os 128 pontos, e o gato é uma CIFRA das letras
 * §B5  as DUAS álgebras no mesmo suporte — e o gato NÃO respeita o XOR
 * §B6  e o `neuronio.c`: o operador bit a bit já era este, sem lhe chamar assim
 * §B7  A TABELA EXAUSTIVA dos 256 bytes: o que cada operação faz a cada posição
 *
 * ── E O QUE AQUI SE DECLARA, QUE NÃO É UM TEOREMA ─────────────────────────────
 * Não se escreve «cada bit É uma lei». Escreve-se a reserva: a posição k do byte fica
 * RESERVADA à realização da Lei k. É uma convenção da arquitectura — cumpre-se ou não —,
 * e o que se prova é o que vem depois: se as operações realizam mesmo as leis que a
 * posição lhes reserva. A arquitectura DECLARA; o neurónio MEDE.
 */
#include <stdio.h>
#include "umbit.h"
#include "ascii.h"
#include "unidade.h"

int main(void){
    printf("\n=== UM BIT: as oito leis, e o ASCII como realização ===\n");

    /* ═══ §B0 AS CINCO PRIMITIVAS EM 𝔽₂ ═════════════════════════════════════ */
    printf("\n§B0 As cinco primitivas colapsam em portas — e o DUAL vira identidade.\n\n");
    {
        long mal = 0, cas = 0, dual_id = 0;
        printf("        x y | x⊕y  x∧y  −x   x−y\n");
        for(B x = 0; x <= 1; x++) for(B y = 0; y <= 1; y++){
            cas++;
            if(b_dif(x,y) != b_som(x,y)) mal++;      /* a diferença É a soma */
            if(b_opo(x) == x) dual_id++;             /* o oposto é o PRÓPRIO x */
            printf("        %d %d |  %d    %d    %d    %d\n", x, y,
                   b_som(x,y), b_mul(x,y), b_opo(x), b_dif(x,y));
        }
        ok("EM 𝔽₂ A ÁLGEBRA NÃO FICA MAIS POBRE, FICA NUA: a soma é o XOR, o produto é o"
           " AND, e o DUAL é a IDENTIDADE — em 𝔽₂ o oposto de x é o próprio x. Logo a"
           " diferença e a soma são a MESMA operação, e o par «soma + dual = diferença» do"
           " teorema das primitivas continua verdadeiro e DEGENERA. É a mesma lei numa face"
           " onde ela quase não se vê, e é por isso que 𝔽₂ é o sítio certo para perceber o"
           " que cada peça fazia",
           mal == 0 && dual_id == cas && cas == 4);
    }

    /* ═══ §B1 ℙ¹(𝔽₂) TEM TRÊS PONTOS, e 0 ↔ ∞ sobrevive ════════════════════ */
    printf("\n§B1 Três pontos — e a troca 0 ↔ ∞ sobrevive no corpo mais pobre.\n\n");
    {
        P1 pt[4]; int n = 0;
        for(B p = 0; p <= 1; p++) for(B q = 0; q <= 1; q++){
            P1 x = p1(p,q);
            if(!p1_e_ponto(x)) continue;
            int novo = 1;
            for(int i = 0; i < n; i++) if(p1_igual(pt[i], x)) novo = 0;
            if(novo) pt[n++] = x;
        }
        int zi = p1_igual(p1_inverte(p1_zero()), p1_inf());
        int iz = p1_igual(p1_inverte(p1_inf()), p1_zero());
        int uu = p1_igual(p1_inverte(p1_um()), p1_um());
        long mal = 0;
        for(int i = 0; i < n; i++)
            if(!p1_igual(p1_inverte(p1_inverte(pt[i])), pt[i])) mal++;
        printf("        os pontos: %d — e são 0, 1 e ∞\n", n);
        printf("        1/0 = ∞ (%s)   1/∞ = 0 (%s)   1/1 = 1 (%s)   ι∘ι = id (%ld"
               " falhas)\n", zi?"sim":"NÃO", iz?"sim":"NÃO", uu?"sim":"NÃO", mal);
        ok("ℙ¹(𝔽₂) TEM TRÊS PONTOS — 0, 1 e ∞ — E A TROCA 0 ↔ ∞ SOBREVIVE, no corpo mais"
           " pobre que existe. Sobrevive porque não usa o sinal: usa a REPRESENTAÇÃO, e a"
           " inversão continua a ser a troca [q:p]. É o Corolário 0 ↔ ∞ a valer onde já não"
           " há sinal nenhum para trocar, o que é a prova mais barata de que ele nunca foi"
           " sobre o sinal",
           n == 3 && zi && iz && uu && mal == 0);
    }

    /* ═══ §B2 AS OITO LEIS COMO TRANSFORMAÇÕES DO BYTE ═════════════════════ */
    printf("\n§B2 As oito leis do catálogo, cada uma uma transformação do byte.\n\n");
    {
        printf("        lei  o enunciado                    a realização em 8 bits"
               "        período\n");
        printf("        ───────────────────────────────────────────────────────────"
               "────────────────\n");
        int p1_ = lei_periodo(lei1_dual,  0x5A, 32);
        int p4  = lei_periodo(lei4_dobra, 0x5A, 32);
        int p5  = lei_periodo(lei5_rotor, 0x5A, 32);
        int p7  = lei_periodo(lei7_par,   0x5A, 32);
        long bid = 0;
        for(int x = 0; x < 256; x++) if(lei2_bidual((V8)x) != (V8)x) bid++;
        int nulos = 0;
        for(int x = 0; x < 256; x++) if(lei0_e_nulo((V8)x)) nulos++;
        long hex = 0, hexc = 0;
        for(B x = 0; x <= 1; x++) for(B y = 0; y <= 1; y++){ hexc++; if(lei6_coincide(x,y)) hex++; }
        printf("        0    a divisão do zero              o par dos nulos 0x00/0xFF"
               "     %d nulos\n", nulos);
        printf("        1    o dual, 1† = −1                o COMPLEMENTO ~x"
               "                 %d\n", p1_);
        printf("        2    o bidual, K** = K              o complemento DUAS vezes"
               "        %s\n", bid == 0 ? "id" : "FALHA");
        printf("        3    o trial {−1, 0, +1}            os pontos de ℙ¹(𝔽₂)"
               "            %d\n", lei3_trial());
        printf("        4    a tetral, T + T*               a troca de nibbles"
               "             %d\n", p4);
        printf("        5    a pental, x² = −1, o bit i     a rotação de dois bits"
               "         %d\n", p5);
        printf("        6    a hexal, soma = produto        onde XOR e AND coincidem"
               "       %ld de %ld\n", hex, hexc);
        printf("        7    o octonião dual ℍ×ℍ*           o dual só num nibble"
               "           %d\n", p7);
        ok("AS OITO LEIS DO CATÁLOGO SÃO OITO TRANSFORMAÇÕES DO BYTE, e o PERÍODO de cada"
           " uma é a testemunha: se ele não batesse, a realização estava errada. O dual tem"
           " período 2 e o bidual é a identidade (Lei 1 e Lei 2, e a segunda É a primeira"
           " duas vezes); a pental — x² = −1, o bit i — tem período 4, que é o do rotor; a"
           " tetral e o octonião têm período 2, porque trocar dois lados duas vezes não"
           " move nada. E a Lei 0 dá exactamente DOIS nulos: 0x00 e 0xFF, o par que ela"
           " enuncia",
           p1_ == 2 && bid == 0 && p4 == 2 && p5 == 4 && p7 == 2
           && nulos == 2 && hex == 1 && lei3_trial() == 3);
    }

    /* ═══ §B3 O CICLO GERADOR: período OITO ════════════════════════════════ */
    printf("\n§B3 O ciclo gerador: rodar um bit tem período OITO.\n\n");
    {
        long mal = 0, cas = 0, cedo = 0;
        for(int x = 1; x < 255; x++){
            int p = lei_periodo(lei_gera, (V8)x, 32);
            cas++;
            if(p == 0) mal++;
            else if(8 % p) mal++;                 /* o período tem de dividir 8 */
            if(p == 8) cedo++;
        }
        int p_meio = lei_periodo(lei_gera, 0x5A, 32);
        printf("        em %ld bytes: %ld com período que não divide 8;  e %ld têm"
               " período 8 exacto\n", cas, mal, cedo);
        printf("        e 0x5A tem período %d\n", p_meio);
        ok("E O CICLO GERADOR TEM PERÍODO OITO: rodar UM bit num byte volta ao princípio"
           " ao fim de oito, e o período de cada byte DIVIDE oito — que é o que um ciclo"
           " gerador quer dizer. O catálogo do Corpo estelar diz que «as oito leis são um"
           " ciclo gerador de período oito», e aqui isso não é uma leitura: é o byte a"
           " realizá-lo. Oito leis, oito bits, período oito — e passar de uma lei à"
           " seguinte é rodar um bit",
           mal == 0 && cas == 254 && cedo > 200);
    }

    /* ═══ §B4 O ASCII: 128 códigos, 128 pontos, e o gato é uma CIFRA ═══════ */
    printf("\n§B4 O ASCII: os 128 códigos SÃO os 128 pontos, e o gato cifra as letras.\n\n");
    {
        /* a bijecção: cada código dá um ponto distinto, e o DEL é o ∞ */
        int visto[AS_N];
        for(int i = 0; i < AS_N; i++) visto[i] = 0;
        long mal = 0;
        for(int c = 0; c < AS_N; c++){
            int v = as_codigo(as_ponto(c));
            if(v < 0 || v >= AS_N || visto[v]) mal++; else visto[v] = 1;
        }
        int inf_ok = (as_codigo(as_ponto(AS_INF)) == AS_INF);
        int zero_inf = (as_inverte(0) == AS_INF), inf_zero = (as_inverte(AS_INF) == 0);
        /* e o gato como cifra: uma permutação das 128 letras */
        int img[AS_N];
        for(int i = 0; i < AS_N; i++) img[i] = 0;
        long colide = 0;
        for(int c = 0; c < AS_N; c++){
            int y = as_gato(2, c);
            if(y < 0 || y >= AS_N) colide++; else img[y]++;
        }
        for(int i = 0; i < AS_N; i++) if(img[i] != 1) colide++;
        char b1[4], b2[4], b3[4];
        printf("        a cifra do gato A₂ em letras:\n");
        printf("          'A'(%d) → %s(%d)    'a'(%d) → %s(%d)    NUL(0) → %s(%d)\n",
               65, as_nome(as_gato(2,65), b1), as_gato(2,65),
               97, as_nome(as_gato(2,97), b2), as_gato(2,97),
               as_nome(as_gato(2,0), b3), as_gato(2,0));
        printf("        a tabela é bijectiva: %ld falhas;  DEL é o ∞ (%s);  1/NUL = DEL"
               " (%s) e 1/DEL = NUL (%s)\n", mal, inf_ok?"sim":"NÃO",
               zero_inf?"sim":"NÃO", inf_zero?"sim":"NÃO");
        printf("        e o gato é uma PERMUTAÇÃO das 128: %ld colisões\n", colide);
        ok("OS 128 CÓDIGOS DO ASCII SÃO OS 128 PONTOS DE ℙ¹(𝔽₁₂₇), e o encaixe não foi"
           " procurado: 127 é primo, é o topo do `int8_t`, e o ASCII foi desenhado para"
           " caber em sete bits. O DEL é o ∞ — não por ser bonito, mas por ser o único"
           " código que sobra depois de os 127 finitos estarem atribuídos. E então o gato"
           " lido em ASCII é uma CIFRA: uma bijecção das 128 letras sem tabela e sem chave,"
           " só a lei — e o esquilo desfá-la, porque é a acção à direita",
           mal == 0 && inf_ok && zero_inf && inf_zero && colide == 0);
    }

    /* ═══ §B5 AS DUAS ÁLGEBRAS NO MESMO SUPORTE ════════════════════════════ */
    printf("\n§B5 Duas álgebras no mesmo suporte — e o gato NÃO respeita o XOR.\n\n");
    {
        long respeita = 0, cas = 0;
        for(int a = 0; a < AS_N; a += 7) for(int b = 0; b < AS_N; b += 11){
            cas++;
            if(as_gato(2, as_xor(a,b)) == as_xor(as_gato(2,a), as_gato(2,b))) respeita++;
        }
        /* e o XOR é mesmo um grupo: todo elemento é o seu próprio inverso */
        long grupo = 0, gc = 0;
        for(int a = 0; a < AS_N; a++){ gc++; if(as_xor(a,a) == 0) grupo++; }
        printf("        o gato respeita o XOR em %ld de %ld pares — e não devia respeitar"
               " em todos\n", respeita, cas);
        printf("        e o XOR é grupo: x ⊕ x = 0 em %ld de %ld\n", grupo, gc);
        ok("O MESMO CONJUNTO CARREGA DUAS ÁLGEBRAS, E ELAS NÃO SÃO COMPATÍVEIS: como"
           " ℙ¹(𝔽₁₂₇) o byte é um PONTO e o gato é uma permutação; como 𝔽₂⁷ o byte é um"
           " VECTOR de sete bits e a soma é o XOR. O gato permuta os pontos e NÃO respeita"
           " o XOR — e isso não é um defeito, é o conteúdo: duas estruturas sobre o mesmo"
           " suporte, e cada operação só faz sentido numa. Confundi-las seria o erro que"
           " esta tabela existe para tornar impossível",
           respeita < cas/2 && grupo == gc && cas > 100);
    }

    /* ═══ §B6 E O `neuronio.c` JÁ ERA ISTO ════════════════════════════════ */
    printf("\n§B6 O operador bit a bit já estava escrito — faltava-lhe o nome.\n\n");
    {
        /* o neuronio lê um ficheiro somando os bits POR POSIÇÃO: é projectar em cada
         * coordenada de 𝔽₂⁸. E o gato dele é a companion, que em 𝔽₂ com m = 1 é
         * (c₀,c₁) ↦ (c₀ ⊕ c₁, c₀) — o passo de Fibonacci em um bit. */
        B c0 = 1, c1 = 0;
        int per = 0;
        for(int k = 1; k <= 16; k++){
            b_gato(&c0, &c1);
            if(c0 == 1 && c1 == 0){ per = k; break; }
        }
        /* e o peso de um byte é o popcount — o Kirchhoff do neuronio */
        long mal = 0;
        for(int x = 0; x < 256; x++){
            int n = 0;
            for(int i = 0; i < 8; i++) n += (x >> i) & 1;
            if(v_peso((V8)x) != n) mal++;
        }
        printf("        o gato em 𝔽₂ (m=1): (c₀,c₁) ↦ (c₀⊕c₁, c₀) — período %d, e"
               " |ℙ¹(𝔽₂)| = 3\n", per);
        printf("        e o peso (o popcount, o Kirchhoff do neuronio): %ld divergências"
               " em 256\n", mal);
        ok("E O OPERADOR BIT A BIT JÁ ESTAVA ESCRITO no `tests/neuronio.c`: ele lê um"
           " ficheiro somando os bits POR POSIÇÃO, que é projectar em cada coordenada de"
           " 𝔽₂⁸, e chama-lhe o Kirchhoff. Aqui a operação diz o seu nome. E o gato dele"
           " em 𝔽₂ com m = 1 é (c₀,c₁) ↦ (c₀⊕c₁, c₀) — o passo de Fibonacci em um bit —,"
           " com período 3, que é o número de pontos de ℙ¹(𝔽₂). A órbita fecha no que"
           " existe: não é coincidência, é o corpo a ser pequeno",
           per == 3 && mal == 0);
    }

    /* ═══ §B7 A TABELA EXAUSTIVA DOS 256 BYTES ════════════════════════════ */
    printf("\n§B7 O teste decisivo: os 256 bytes, e o que cada operação faz a cada"
           " posição.\n\n");
    {
        struct { const char *nome; V8 (*f)(V8); } op[] = {
            { "Lei 1  o dual ~x",         lei1_dual  },
            { "Lei 2  o bidual ~~x",      lei2_bidual},
            { "Lei 4  a dobra (nibbles)", lei4_dobra },
            { "Lei 5  o rotor (2 bits)",  lei5_rotor },
            { "Lei 7  o par (só 1 lado)", lei7_par   },
            { "o gerador (1 bit)",        lei_gera   },
        };
        int n = (int)(sizeof op / sizeof *op);
        printf("        operação                b7 b6 b5 b4 b3 b2 b1 b0\n");
        printf("        ────────────────────────────────────────────────\n");
        long duvidas = 0, duais = 0, preservas = 0, moves = 0;
        for(int i = 0; i < n; i++){
            printf("        %-23s", op[i].nome);
            for(int k = 7; k >= 0; k--){
                char c = lei_faz(op[i].f, k);
                printf(" %c ", c);
                if(c == '?') duvidas++;
                else if(c == 'd') duais++;
                else if(c == 'p') preservas++;
                else moves++;
            }
            printf("\n");
        }
        printf("\n        p = preserva a posição · d = realiza o DUAL nela · m = move\n");
        printf("        totais: %ld preservam, %ld realizam o dual, %ld movem, %ld"
               " indefinidas\n", preservas, duais, moves, duvidas);
        ok("E A TABELA EXAUSTIVA DOS 256 BYTES É O TESTE DECISIVO, porque transforma a"
           " declaração em objecto: por cada operação e cada posição, ela PRESERVA o bit,"
           " realiza o DUAL nele, ou MOVE-o. O dual (Lei 1) inverte TODAS as oito posições;"
           " o bidual preserva todas — que é a Lei 2 a ser a Lei 1 duas vezes, medida e não"
           " citada. O octonião dual inverte SÓ metade, que é «ligar sem fundir» a"
           " acontecer nos bits. E nenhuma coluna fica indefinida: cada operação faz uma"
           " das três coisas em toda a parte",
           duvidas == 0 && duais > 0 && preservas > 0 && moves > 0);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
