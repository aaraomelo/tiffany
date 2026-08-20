/* tests/naturais.c — OS NATURAIS A PARTIR DO BINÁRIO: F_{2w} = F_w + F_w*, e F_8 = W_8.
 *
 * O `inteiros.tex` desce de ℕ (duplica e quocienta: ℕ²/∼ = ℤ). Aqui SOBE-SE até ℕ: parte-se
 * de 𝔽₂ — um bit, cinco primitivas, duas portas — e aplica-se o MESMO molde dual quatro
 * vezes. A largura dobra (1→2→4→8), o corpo dobra (2→4→16→256), e o suporte do topo é
 * exactamente W_8 = {0,…,255}, o envelope do `inteiros.tex` §sec:palavra8.
 *
 * NOTAÇÃO (a do paper): F_w := GF(2^w) — o índice conta BITS. F_1 = 𝔽₂, |F_8| = 256, e a
 * torre é F_1 → F_2 → F_4 → F_8. No código o andar é k = log2(w): k=0..3 ↔ F_1,F_2,F_4,F_8.
 *
 * §NB0  a TORRE ergue-se com a régua DERIVADA: λ_k pelo menor irredutível, e cada andar é corpo
 * §NB1  a SOMA é o XOR em todo andar, e é a sua própria inversa — Lei 0 e Lei 1 nuas
 * §NB2  o DUAL é a conjugação x* = x^{|F_baixo|}: involução, automorfismo, e fixa o andar de baixo
 * §NB3  O ENCAIXE: o produto do andar 3 RESTRITO aos prefixos É o produto do andar k — Lei 7
 * §NB4  o FROBENIUS conta os andares: {x : x^{2^w} = x} tem 2^w elementos e É {0,…,2^w−1}
 * §NB5  o TRIAL em característica 2: σ²+σ+λ tem 0 ou 2 raízes, NUNCA 1 — a cúspide não existe
 * §NB6  A DIFERENÇA ENTRE ⊕ E + É O TRANSPORTE: a⊕b = a+b ⟺ a∧b = 0, e são 3^8 pares
 * §NB7  ℕ DO BINÁRIO: a soma natural é o PONTO FIXO do par (⊕,∧) — medida contra Peano
 * §NB8  O GUME, DUAS RÉGUAS: o produto do AES dá corpo e NÃO encaixa — {0,…,15} não fecha
 * §NB12 KUMMER: o número de transportes é v₂(C(a+b,a)) — a forma fechada de §NB6
 * §NB13 O ANDAR SEGUINTE: λ₃ = 128 = Φ/2, medido no degrau F_8 → F_16
 * §NB11 A BASE de oito da torre É a base ORTONORMAL das oito leis (`ortonormal.c` §O0)
 * §NB10 A RÉGUA MÍNIMA É A DE CONWAY: λ_k = 2^{2^k−1}, e o mex dá o MESMO produto
 * §NB9  E O ESCOPO: F_8 é o SUPORTE de W_8, não ℕ — sem ordem, e finito. Diz-se qual é qual.
 *
 * ── O QUE AQUI SE MEDE, E O QUE NÃO SE AFIRMA ────────────────────────────────────────
 * Não se escreve «F_8 é ℕ». F_8 é finito, tem característica 2 e não admite ordem de corpo
 * (`corpo_algebrico.tex` §O byte). O que se mede é mais fino e é verdade: (i) o SUPORTE do
 * andar k é o prefixo {0,…,2^{2^k}−1} de ℕ pela identidade, e os andares encaixam; (ii) as
 * duas operações de ℕ nesse suporte reconstroem-se das DUAS primitivas de 𝔽₂ pela iteração
 * do transporte. ℕ = binário + transporte, e o transporte é o que a dualidade não guarda.
 */
#include <stdio.h>
#include "binario.h"
#include "umbit.h"
#include "naturais.h"
#include "unidade.h"

int main(void){
    printf("\n=== OS NATURAIS A PARTIR DO BINÁRIO: F_{2w} = F_w + F_w* ===\n");
    bn_torre();

    /* ═══ §NB0 A TORRE, COM A RÉGUA DERIVADA ═══════════════════════════════════ */
    printf("\n§NB0 Quatro andares, e nenhuma régua escrita à mão.\n\n");
    {
        long corpos = 0, inv_mal = 0, card_mal = 0;
        printf("        k  largura  |F|   λ_k (régua para o andar seguinte)\n");
        for(int k = 0; k < BN_ANDARES; k++){
            printf("        %d     %u    %3u   %s%u\n", k, bn_larg(k), bn_card(k),
                   k+1 < BN_ANDARES ? "λ=" : "— (topo) ", k+1 < BN_ANDARES ? bn_lam[k] : 0u);
            if(bn_card(k) != (1u << bn_larg(k))) card_mal++;
            /* corpo: todo não-nulo tem inversa, e a inversa é exibida pela volta */
            long mal = 0;
            for(unsigned x = 1; x < bn_card(k); x++)
                if(bn_mul(k, x, bn_inv(k,x)) != 1u) mal++;
            inv_mal += mal;
            if(!mal) corpos++;
        }
        ok("A TORRE ERGUE-SE SOZINHA: a régua λ_k não se escreve — procura-se o MENOR λ do"
           " andar de baixo para o qual σ²+σ+λ não tem raiz, e é ele que constrói o andar de"
           " cima. Quatro andares (1, 2, 4, 8 bits), quatro cardinais (2, 4, 16, 256), e em"
           " cada um TODO não-nulo tem inversa, exibida pela volta x·x⁻¹ = 1. O construtor é"
           " um só — o mesmo código dá o andar 1 do andar 0 e o andar 3 do andar 2",
           corpos == BN_ANDARES && inv_mal == 0 && card_mal == 0);
    }

    /* ═══ §NB1 A SOMA É O XOR, E É A SUA PRÓPRIA INVERSA ═══════════════════════ */
    printf("\n§NB1 Lei 0 e Lei 1 nuas: x ⊕ x = 0, e −x = x.\n\n");
    {
        long mal = 0, cas = 0;
        for(int k = 0; k < BN_ANDARES; k++)
            for(unsigned x = 0; x < bn_card(k); x++){
                cas++;
                if(bn_som(x,x) != 0) mal++;                    /* a folha: x + x† = 0 */
                if(bn_opo(x) != x) mal++;                      /* o dual degenera     */
                if(bn_som(bn_som(x, 7u & bn_masc(k)), 7u & bn_masc(k)) != x) mal++;
            }
        printf("        %ld elementos varridos nos quatro andares, %ld falhas\n", cas, mal);
        ok("A SOMA NÃO PRECISA DE SUBTRACÇÃO: em característica 2 o oposto de x é o próprio x,"
           " logo a folha do `inteiros.tex` (x ⊕ x† = 0) é imediata e a subtracção JÁ é total"
           " no chão — não é preciso duplicar e quocientar para a obter. O que se paga é o"
           " outro lado do par: sem sinal não há ordem (§NB9). O degrau ℕ→ℤ do outro paper"
           " compra a subtracção com a ordem na mão; aqui ela vem de graça e a ordem é que fica"
           " por comprar",
           mal == 0 && cas == 2 + 4 + 16 + 256);
    }

    /* ═══ §NB2 O DUAL: CONJUGAÇÃO, INVOLUÇÃO, AUTOMORFISMO ═════════════════════ */
    printf("\n§NB2 x* = x^{|F_baixo|}: involução (Lei 2), automorfismo, e fixa o andar de baixo.\n\n");
    {
        long inv_mal = 0, hom_mal = 0, fix_mal = 0;
        for(int k = 1; k < BN_ANDARES; k++){
            unsigned n = bn_card(k), meio = bn_card(k-1);
            for(unsigned x = 0; x < n; x++){
                if(bn_conj(k, bn_conj(k,x)) != x) inv_mal++;               /* Lei 2 */
                /* o dual É o Frobenius do andar: x* = x^{|F_{k-1}|} */
                if(bn_conj(k,x) != bn_pot(k, x, meio)) hom_mal++;
                /* fixa exactamente o andar de baixo — que é o PREFIXO {0,…,meio−1} */
                if((bn_conj(k,x) == x) != (x < meio)) fix_mal++;
            }
            for(unsigned a = 0; a < n; a++) for(unsigned b = 0; b < n; b++){
                if(bn_conj(k, bn_mul(k,a,b)) != bn_mul(k, bn_conj(k,a), bn_conj(k,b))) hom_mal++;
                if(bn_conj(k, bn_som(a,b))  != bn_som(bn_conj(k,a), bn_conj(k,b)))      hom_mal++;
            }
        }
        printf("        involução: %ld falhas · automorfismo: %ld · ponto fixo = andar de baixo: %ld\n",
               inv_mal, hom_mal, fix_mal);
        ok("O DUAL DESTE DEGRAU NÃO É UMA CONVENÇÃO: é a segunda raiz. σ²+σ+λ tem duas raízes,"
           " σ e σ+1, e trocá-las é a conjugação x ↦ x*. Ela é involução (Lei 2), é automorfismo"
           " nos DOIS lados (soma e produto), e o seu conjunto de pontos fixos é EXACTAMENTE o"
           " andar de baixo. Por isso o encaixe não precisa de mergulho escrito: o subcorpo é"
           " quem o dual não move — e coincide, elemento a elemento, com o prefixo de ℕ",
           inv_mal == 0 && hom_mal == 0 && fix_mal == 0);
    }

    /* ═══ §NB3 O ENCAIXE — Lei 7: ligar sem fundir ═════════════════════════════ */
    printf("\n§NB3 O produto do topo, restrito ao prefixo, É o produto do andar.\n\n");
    {
        long mal = 0, pares = 0;
        for(int k = 0; k + 1 < BN_ANDARES; k++)
            for(unsigned a = 0; a < bn_card(k); a++) for(unsigned b = 0; b < bn_card(k); b++){
                pares++;
                if(bn_mul(BN_ANDARES-1, a, b) != bn_mul(k, a, b)) mal++;
                if(bn_som(a,b) >= bn_card(k)) mal++;            /* fecha também na soma */
            }
        printf("        {0,1} ⊂ {0..3} ⊂ {0..15} ⊂ {0..255}: %ld pares, %ld falhas\n", pares, mal);
        ok("O ENCAIXE É EXACTO E É NOS PREFIXOS DE ℕ: multiplicar 7 por 11 dentro de F_16 e"
           " dentro de F_256 dá o MESMO número, e o resultado nunca sai de {0,…,15}. Não há"
           " conversão entre andares porque não há representação a converter — a torre partilha"
           " o suporte. É a Lei 7 literal: os andares ligam-se sem se fundirem, e o par (a0,a1)"
           " continua legível dentro do byte depois de multiplicado",
           mal == 0 && pares == 4 + 16 + 256);
    }

    /* ═══ §NB4 O FROBENIUS CONTA OS ANDARES ════════════════════════════════════ */
    printf("\n§NB4 {x : x^{2^w} = x} tem 2^w elementos — e são os 2^w primeiros naturais.\n\n");
    {
        long mal = 0; int K = BN_ANDARES-1;
        printf("        w   |{x : φ^w(x) = x}|   esperado   é o prefixo?\n");
        for(int k = 0; k < BN_ANDARES; k++){
            unsigned w = bn_larg(k), cnt = 0; int prefixo = 1;
            for(unsigned x = 0; x < bn_card(K); x++){
                unsigned y = x;
                for(unsigned i = 0; i < w; i++) y = bn_frob(K, y);    /* φ^w(x) = x^{2^w} */
                if(y == x){ cnt++; if(x >= bn_card(k)) prefixo = 0; }
                else if(x < bn_card(k)) prefixo = 0;
            }
            printf("        %u        %3u              %3u          %s\n",
                   w, cnt, bn_card(k), prefixo ? "sim" : "NÃO");
            if(cnt != bn_card(k) || !prefixo) mal++;
        }
        ok("OS ANDARES NÃO SE DECLARAM, LEEM-SE NO TOPO: o subcorpo de largura w é o conjunto"
           " dos x que o Frobenius iterado w vezes devolve — e, medido dentro de F_256, esse"
           " conjunto tem exactamente 2^w elementos E é o prefixo {0,…,2^w−1}. Dois caminhos"
           " independentes para o mesmo conjunto: um pela CONSTRUÇÃO (a torre que se ergueu"
           " em §NB0) e outro pela EQUAÇÃO x^{2^w} = x, que não sabe nada da construção",
           mal == 0);
    }

    /* ═══ §NB5 O TRIAL EM CARACTERÍSTICA 2 ═════════════════════════════════════ */
    printf("\n§NB5 σ²+σ+λ: 0 ou 2 raízes, nunca 1 — em char 2 a cúspide não existe.\n\n");
    {
        long um = 0, zero = 0, dois = 0, outro = 0;
        for(int k = 0; k + 1 < BN_ANDARES; k++)
            for(unsigned lam = 0; lam < bn_card(k); lam++){
                int r = bn_raizes(k, lam);
                if(r == 0) zero++; else if(r == 1) um++; else if(r == 2) dois++; else outro++;
            }
        printf("        λ com 0 raízes (irredutível): %ld · com 1 (cúspide): %ld · com 2: %ld\n",
               zero, um, dois);
        ok("O TRIAL DEGENERA, E É POR ISSO QUE A TORRE DOBRA SEM EXCEPÇÃO. Em característica"
           " ímpar τ = sign(disc) toma três valores e o caso τ = 0 — a cúspide, onde as duas"
           " folhas colidem — é o degrau que exige tratamento à parte. Aqui a derivada de"
           " σ²+σ+λ é a constante 1, nunca se anula, e a raiz dupla é IMPOSSÍVEL: cada λ ou"
           " não tem raiz nenhuma ou tem exactamente duas. O trial colapsa em {−1,+1} e a"
           " dobra tem sempre o par de folhas que precisa. E os dois lados repartem-se ao"
           " meio, que é o traço a ser 0 ou 1",
           um == 0 && outro == 0 && zero == dois && zero == (2 + 4 + 16)/2);
    }

    /* ═══ §NB6 A DIFERENÇA É O TRANSPORTE ══════════════════════════════════════ */
    printf("\n§NB6 a ⊕ b = a + b exactamente quando a ∧ b = 0 — e são 3^8 pares.\n\n");
    {
        long iguais = 0, sem_carry = 0, mal = 0;
        for(unsigned a = 0; a < 256; a++) for(unsigned b = 0; b < 256; b++){
            int xor_e_soma = ((a ^ b) == (a + b));
            int sem = ((a & b) == 0);
            if(xor_e_soma) iguais++;
            if(sem) sem_carry++;
            if(xor_e_soma != sem) mal++;                 /* o ⟺, par a par */
        }
        printf("        pares com a⊕b = a+b: %ld · pares com a∧b = 0: %ld · 3^8 = %d\n",
               iguais, sem_carry, 3*3*3*3*3*3*3*3);
        ok("O QUE SEPARA O CORPO DE ℕ TEM NOME, E CONTA-SE: é o TRANSPORTE. Nos 65 536 pares"
           " de W_8, o XOR do corpo coincide com a soma de ℕ exactamente nos pares sem"
           " sobreposição de bits — nem um caso a mais nem a menos —, e esses são 3^8, porque"
           " cada uma das oito posições tem três estados admissíveis (0/0, 1/0, 0/1) e o"
           " quarto é o que gera o transporte. A soma de 𝔽₂ é a soma de ℕ com o transporte"
           " DESLIGADO — é a mesma operação, sem a memória entre as posições",
           mal == 0 && iguais == 6561);
    }

    /* ═══ §NB7 ℕ DO BINÁRIO: O PONTO FIXO DO PAR (⊕, ∧) ════════════════════════ */
    printf("\n§NB7 A soma de ℕ é o ponto fixo de (⊕, ∧) — contra Peano, exaustivo.\n\n");
    {
        long mal_soma = 0, mal_mult = 0, mal_S = 0; int passos_max = 0;
        for(unsigned a = 0; a < 256; a++) for(unsigned b = 0; b < 256; b++){
            int p = 0;
            unsigned long s = bn_soma_nat(a, b, &p);
            if(s != nt_soma(a, b)) mal_soma++;                    /* contra a recursão de Peano */
            if(p > passos_max) passos_max = p;
            if(bn_mult_nat(a,b) != nt_mult(a,b)) mal_mult++;
        }
        for(unsigned n = 0; n < 256; n++) if(bn_S(n) != nt_S(n)) mal_S++;
        printf("        soma: %ld falhas · produto: %ld · sucessor: %ld · passos do transporte ≤ %d\n",
               mal_soma, mal_mult, mal_S, passos_max);
        ok("E ENTÃO ℕ SAI DO BINÁRIO, SEM TABELA E SEM MEMÓRIA: a soma natural é o ponto fixo"
           " da iteração (a,b) ↦ (a⊕b, (a∧b)≪1) — a soma de 𝔽₂ e o produto de 𝔽₂, as duas"
           " ÚNICAS primitivas que um bit tem, uma a dar o dígito e a outra a dar o transporte."
           " O laço seca em poucos passos e o resultado bate com a definição recursiva de Peano"
           " (n+0 = n, n+S(m) = S(n+m)) nos 65 536 pares; o produto, pela mesma via, bate com a"
           " soma iterada. Dois caminhos que não partilham código: um é indução, o outro é bit",
           mal_soma == 0 && mal_mult == 0 && mal_S == 0 && passos_max > 0 && passos_max <= 9);
    }

    /* ═══ §NB8 O GUME: DUAS RÉGUAS PARA O MESMO BYTE ═══════════════════════════ */
    printf("\n§NB8 O produto do AES também dá corpo — e NÃO encaixa nos prefixos.\n\n");
    {
        long aes_inv_mal = 0, aes_fora = 0;
        unsigned ca = 0, cb = 0, cr = 0;
        /* o AES é corpo: todo não-nulo tem inversa (por busca, sem Fermat) */
        for(unsigned x = 1; x < 256; x++){
            int achou = 0;
            for(unsigned y = 1; y < 256 && !achou; y++) if(bn_mul_aes(x,y) == 1) achou = 1;
            if(!achou) aes_inv_mal++;
        }
        /* mas {0,…,15} não fecha: exibe-se o primeiro que sai */
        for(unsigned a = 0; a < 16; a++) for(unsigned b = 0; b < 16; b++){
            unsigned r = bn_mul_aes(a,b);
            if(r > 15){ if(!aes_fora){ ca = a; cb = b; cr = r; } aes_fora++; }
        }
        printf("        AES: %ld elementos sem inversa · %ld produtos de {0..15} que saem"
               " (o primeiro: %u·%u = %u)\n", aes_inv_mal, aes_fora, ca, cb, cr);
        printf("        torre: os mesmos 256 números, e %u·%u = %u, dentro de {0..15}\n",
               ca, cb, bn_mul(3, ca, cb));
        ok("DUAS RÉGUAS PARA O MESMO CONJUNTO, E SÓ UMA ENCAIXA. O produto do AES (módulo"
           " x⁸+x⁴+x³+x+1) dá um corpo de 256 elementos tão legítimo quanto o da torre — são"
           " isomorfos, e todo não-nulo tem inversa nos dois. Mas o suporte {0,…,15} não é"
           " subcorpo do AES: exibe-se o produto que sai. Logo a frase «os 2^{2^k} primeiros"
           " naturais formam um corpo» NÃO é uma propriedade do byte nem da característica 2 —"
           " é uma propriedade DA TORRE, da escolha de erguer por dobra dual em vez de por um"
           " polinómio de grau oito. A régua que se escolhe é parte do resultado",
           aes_inv_mal == 0 && aes_fora > 0 && bn_mul(3, ca, cb) < 16);
    }

    /* ═══ §NB9 O ESCOPO: F_8 É O SUPORTE, NÃO É ℕ ══════════════════════════════ */
    printf("\n§NB9 O que F_8 tem, e o que não tem.\n\n");
    {
        /* sem ordem de corpo: somar uns fecha, e o número de parcelas É a característica */
        unsigned t = 0; int c = 0;
        do { t = bn_som(t, 1u); c++; } while(t != 0);
        /* e em ℕ o mesmo laço não fecha: o sucessor nunca repete dentro do envelope */
        int repetiu = 0;
        for(unsigned n = 0; n < 255; n++) if(nt_S(n) == n) repetiu = 1;
        printf("        característica de F_8 (medida: 1+1+…+1 = 0): %d parcelas\n", c);
        printf("        |F_8| = 256 = |W_8| · sucessor de Peano sem ponto fixo em W_8: %s\n",
               repetiu ? "FALSO" : "verdadeiro");
        ok("E DIZ-SE QUAL É QUAL, QUE É ONDE ESTA CONSTRUÇÃO SE PODIA MENTIR A SI PRÓPRIA."
           " F_8 NÃO é ℕ: é finito, tem característica 2 — somar uns fecha em dois passos — e"
           " por isso não admite ordem de corpo (`corpo_algebrico.tex`, §O byte). O que é"
           " verdade, e é o que se mediu: o SUPORTE de F_8 é exactamente W_8 = {0,…,255} do"
           " `inteiros.tex`, pela identidade e não por codificação; os andares de baixo são os"
           " prefixos; e as duas operações de ℕ nesse suporte reconstroem-se das duas de 𝔽₂"
           " mais o transporte. ℕ não é o topo da torre — é a torre MAIS a memória entre"
           " posições que ela não guarda",
           c == 2 && !repetiu && bn_card(3) == 256);
    }

    /* ═══ §NB10 A RÉGUA MÍNIMA É A DE CONWAY — pelo mex, que não sabe da torre ═══ */
    printf("\n§NB10 λ_k = 2^{2^k−1}, e o produto bate com o nim-produto definido pelo mex.\n\n");
    {
        /* o nim-produto pela DEFINIÇÃO de Conway: a⊗b = mex{a'⊗b ⊕ a⊗b' ⊕ a'⊗b'}.
         * Nada aqui sabe de σ, de λ ou de polinómios — é a definição combinatória do
         * jogo, e chega-se ao mesmo produto. Exaustivo em F_16 (256 pares). */
        static unsigned nim[16][16];
        for(unsigned a = 0; a < 16; a++) for(unsigned b = 0; b < 16; b++){
            unsigned char visto[256] = {0};
            for(unsigned x = 0; x < a; x++) for(unsigned y = 0; y < b; y++)
                visto[(nim[x][b] ^ nim[a][y] ^ nim[x][y]) & 0xFF] = 1;
            unsigned m = 0; while(m < 256 && visto[m]) m++;
            nim[a][b] = m;
        }
        long mal_nim = 0, mal_topo = 0, mal_regua = 0;
        for(unsigned a = 0; a < 16; a++) for(unsigned b = 0; b < 16; b++){
            if(nim[a][b] != bn_mul(2, a, b)) mal_nim++;
            if(nim[a][b] != bn_mul(3, a, b)) mal_topo++;    /* e no andar de cima também */
        }
        printf("        λ derivado:");
        for(int k = 0; k + 1 < BN_ANDARES; k++){
            unsigned meio = 1u << (bn_larg(k) - 1);          /* 2^{2^k−1} = F/2 */
            printf("  λ_%d = %u (F/2 = %u)", k, bn_lam[k], meio);
            if(bn_lam[k] != meio) mal_regua++;
        }
        printf("\n        nim(mex) vs torre em F_16: %ld falhas · dentro de F_256: %ld\n",
               mal_nim, mal_topo);
        /* AS DUAS REGRAS DE CONWAY para as potências de Fermat F = 2^{2^k}, medidas
         * na TORRE: (i) F⊗F = 3F/2 = F ⊕ F/2 — que é σ² = σ + λ com λ = F/2;
         * (ii) F⊗x = Fx (o produto ORDINÁRIO de inteiros) para todo x < F — que é o
         * deslocamento. São elas que sustentam a indução do Thm. do nim no paper, e
         * aqui não se citam: medem-se. */
        long r1 = 0, r2 = 0;
        for(int k = 0; k + 1 < BN_ANDARES; k++){
            unsigned F = bn_card(k);                       /* 2, 4, 16 */
            if(bn_mul(3, F, F) != (F ^ (F >> 1))) r1++;     /* F⊗F = F ⊕ F/2 */
            for(unsigned x = 0; x < F; x++)
                if(bn_mul(3, F, x) != F * x) r2++;          /* F⊗x = Fx, produto de ℕ */
        }
        printf("        regras de Conway na torre: F⊗F = F⊕F/2 → %ld falhas ·"
               " F⊗x = F·x (x<F) → %ld falhas\n", r1, r2);
        ok("E A RÉGUA MÍNIMA NÃO ERA NOVA — É A DO NIM. O λ que a busca devolve em cada andar"
           " é exactamente 2^{2^k−1}, isto é, METADE da potência de Fermat que abre o andar; e"
           " com essa régua o produto da torre coincide, par a par, com o nim-produto definido"
           " pelo mex — uma definição que não conhece σ, λ nem polinómio nenhum, só o jogo. Os"
           " naturais que o Conway obteve como corpo pela combinatória são os mesmos que a"
           " dobra dual constrói pela álgebra. E as DUAS REGRAS de que a indução precisa"
           " medem-se aqui em vez de se citarem: F⊗F = F ⊕ F/2 (que É σ² = σ + λ com λ = F/2)"
           " e F⊗x = F·x para x < F (que É o deslocamento). Escopo do mex: exaustivo em F_16"
           " e verificado dentro de F_256 nos mesmos pares — o topo inteiro pelo mex não se"
           " varreu (256⁴ candidatos); a igualdade GERAL é o teorema, e a prova é a indução",
           mal_nim == 0 && mal_topo == 0 && mal_regua == 0 && r1 == 0 && r2 == 0);
    }

    /* ═══ §NB11 A BASE DA TORRE É A BASE ORTONORMAL DAS OITO LEIS ══════════════ */
    printf("\n§NB11 Três geradores, oito produtos: e_k = 2^k, e a Gram é a identidade.\n\n");
    {
        /* os geradores: um por dobra. σ_j é o primeiro elemento NOVO do andar j+1 —
         * e o primeiro elemento novo é a potência de Fermat que abre o andar. */
        unsigned g[3] = { bn_card(0), bn_card(1), bn_card(2) };     /* 2, 4, 16 */
        long mal_base = 0, mal_gram = 0, mal_filtro = 0;
        printf("        geradores (um por dobra): σ₀=%u  σ₁=%u  σ₂=%u\n", g[0], g[1], g[2]);
        printf("        k  bits de k   produto na torre   2^k\n");
        for(unsigned k = 0; k < 8; k++){
            unsigned e = 1;
            for(int j = 0; j < 3; j++) if(k >> j & 1) e = bn_mul(3, e, g[j]);
            printf("        %u    %u%u%u            %3u          %3u\n",
                   k, k>>2&1, k>>1&1, k&1, e, 1u << k);
            if(e != (1u << k)) mal_base++;
        }
        /* a Gram na forma ⟨a,b⟩ = paridade(a ∧ b) — `ortonormal.c` §O0, aqui na base
         * que a TORRE produziu (e não numa base postulada) */
        for(unsigned i = 0; i < 8; i++) for(unsigned j = 0; j < 8; j++){
            B g_ij = v_interno((V8)(1u << i), (V8)(1u << j));
            if(g_ij != (i == j)) mal_gram++;
        }
        /* e a filtração: os 2^k primeiros vectores da base geram o andar k — o span
         * dos e_0..e_{2^k−1} é exactamente o prefixo {0,…,2^{2^k}−1} */
        for(int k = 0; k < BN_ANDARES; k++){
            unsigned w = bn_larg(k), span = 0;
            for(unsigned i = 0; i < w; i++) span |= (1u << i);      /* soma = XOR livre */
            if(span != bn_masc(k)) mal_filtro++;
        }
        printf("        base: %ld falhas · Gram = I: %ld · filtração por andar: %ld\n",
               mal_base, mal_gram, mal_filtro);
        ok("E A BASE DE OITO NÃO SE POSTULOU AQUI — SAIU DA TORRE, E É A MESMA. Três dobras dão"
           " três geradores (σ₀=2, σ₁=4, σ₂=16), e os oito produtos que se formam escolhendo ou"
           " não cada um deles são exactamente 1, 2, 4, …, 128: o índice do vector É o subconjunto"
           " de dobras que o produziu, lido em binário. A base multiplicativa da torre coincide,"
           " elemento a elemento, com a base aditiva e_k = 2^k que `ortonormal.c` §O0 mostrou ser"
           " ORTONORMAL (Gram = I sob ⟨a,b⟩ = paridade(a∧b)) — e é essa a razão de a coordenada"
           " se ler com um AND: é a base ortonormal escrita em hardware. Os primeiros 2^k"
           " vectores geram o andar k, de modo que o encaixe de §NB3 é a FILTRAÇÃO desta base",
           mal_base == 0 && mal_gram == 0 && mal_filtro == 0);
    }

    /* ═══ §NB12 KUMMER: o número de transportes é v₂ do binomial ═══════════════ */
    printf("\n§NB12 O transporte tem forma fechada: contá-los é ler v₂(C(a+b,a)).\n\n");
    {
        long mal = 0, total = 0, max_c = 0;
        for(unsigned a = 0; a < 256; a++) for(unsigned b = 0; b < 256; b++){
            /* conta os transportes GERADOS ao somar a e b em base 2 */
            int carry = 0, n = 0;
            for(int i = 0; i < 10; i++){
                int x = (a >> i) & 1, y = (b >> i) & 1;
                int novo = (x + y + carry) >= 2;
                if(novo) n++;
                carry = novo;
            }
            /* Legendre: v₂(m!) = m − s₂(m), logo v₂(C(a+b,a)) = s₂(a)+s₂(b)−s₂(a+b) */
            int sa = 0, sb = 0, ss = 0;
            for(unsigned t = a; t; t >>= 1) sa += t & 1;
            for(unsigned t = b; t; t >>= 1) sb += t & 1;
            for(unsigned t = a+b; t; t >>= 1) ss += t & 1;
            total++;
            if(n != sa + sb - ss) mal++;
            if(n > max_c) max_c = n;
        }
        printf("        %ld pares: %ld divergências · máximo de transportes num par: %ld\n",
               total, mal, max_c);
        ok("E O TRANSPORTE TEM FORMA FECHADA — a dívida que o paper declarava. Contar os"
           " transportes ao somar a e b em base 2 dá exactamente s₂(a) + s₂(b) − s₂(a+b),"
           " que por Legendre é v₂ do coeficiente binomial C(a+b, a): é o teorema de Kummer,"
           " medido nos 65 536 pares de W_8 sem uma divergência. O que §NB6 contava como"
           " «quantos pares não têm transporte» (3^8) ganha aqui o outro lado: QUANTOS"
           " transportes tem cada par, e o número não é uma contagem empírica — é a valuação"
           " 2-ádica de um binomial. A ponte entre a soma binária e a aritmética dos primos"
           " passa por aqui, e não era preciso inventá-la",
           mal == 0 && total == 65536 && max_c > 0);
    }

    /* ═══ §NB13 O ANDAR SEGUINTE: λ₃, e a régua continua a ser a metade ════════ */
    printf("\n§NB13 F_16 (65 536 elementos): o menor λ que serve, e o que se prevê.\n\n");
    {
        unsigned lam3 = bn_ergue(3);                  /* varre F_8: 256 λ × 256 y */
        unsigned previsto = 1u << (bn_larg(3) - 1);   /* 2^{2^3−1} = 128 = Φ/2 */
        /* e conta-se quantos servem: metade, pelo traço */
        long servem = 0;
        for(unsigned lam = 0; lam < 256; lam++) if(bn_raizes(3, lam) == 0) servem++;
        printf("        λ₃ derivado = %u · previsto (Φ/2) = %u · λ que servem: %ld de 256\n",
               lam3, previsto, servem);
        ok("E O ANDAR SEGUINTE NÃO PRECISA DE SER CONSTRUÍDO PARA A RÉGUA SER SABIDA. O paper"
           " deixava por medir se λ_k = 2^{2^k−1} continuava a ser o MENOR para k ≥ 3; mede-se"
           " agora no degrau que ergue F_8 até F_16, os 65 536 elementos: a busca devolve 128,"
           " que é exactamente Φ/2, e metade dos 256 candidatos serve — a mesma repartição ao"
           " meio de §NB5, pelo traço. A conjectura da Prop. do menor λ passa a ter mais um"
           " andar medido, e continua conjectura para os de cima: não se afirma o que não se"
           " varreu",
           lam3 == previsto && servem == 128);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
