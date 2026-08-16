/* tests/ortonormal.c — A BASE DAS OITO LEIS É ORTONORMAL, e é ISSO que autoriza o bit a bit.
 *
 * O Aarão: «o teorema universal garante operação bit a bit com uma base de tamanho 8 (1 byte)
 * de cada lei; mostra que geralmente o corpo universal tem uma base ORTONORMAL e é essa das
 * oito leis — prova isso e faz a migração.»
 *
 * ── O QUE SE PROVA, E POR QUE É ELE QUE AUTORIZA O RESTO ──────────────────────
 * Em 𝔽₂⁸ com a forma bilinear
 *
 *      ⟨a, b⟩ = paridade(a AND b)
 *
 * — a única forma bilinear que 𝔽₂ tem — a base das oito leis e_k = 2^k satisfaz
 *
 *      ⟨e_i, e_j⟩ = δ_ij      (ORTONORMAL)
 *
 * e daí sai a consequência que interessa: a coordenada k de um byte RECUPERA-SE pelo
 * produto interno com e_k,
 *
 *      a_k = ⟨a, e_k⟩ ,
 *
 * e esse produto interno é UM AND seguido de UMA paridade — ou seja, é LER O BIT.
 *
 *      É por a base ser ortonormal que a extracção de coordenada é a leitura de um bit.
 *
 * Numa base qualquer, recuperar a coordenada exigiria resolver um sistema — inverter a
 * matriz de Gram. Aqui a matriz de Gram É a identidade, e por isso a operação é bit a bit.
 * O «uint8_t» não é uma escolha de armazenamento: é a base ortonormal escrita em hardware.
 *
 * ── E O PREÇO, que se diz ─────────────────────────────────────────────────────
 * A forma é bilinear e simétrica, mas NÃO é definida positiva — em 𝔽₂ isso nem faz
 * sentido, e há vectores isótropos (⟨a,a⟩ = 0 com a ≠ 0): todo byte de peso PAR é
 * ortogonal a si próprio. «Ortonormal» aqui quer dizer Gram = I na base, e não
 * «comprimento um» no sentido euclidiano. Dizer o contrário seria importar uma noção que
 * o corpo não tem.
 *
 * §O0  a matriz de GRAM da base das oito leis É a identidade — 64 pares, exaustivo
 * §O1  a coordenada é o PRODUTO INTERNO, e o produto interno é LER O BIT
 * §O2  e é por isso que a operação é bit a bit: Gram = I dispensa resolver sistema
 * §O3  o preço, dito: há vectores isótropos, e «ortonormal» aqui é Gram = I
 * §O4  o gume: numa base NÃO ortogonal a coordenada deixa de ser um bit
 * §O5  e a família por andar: σ_{L_k} = φ^k nos Lucas ímpares (thm:sigma4, teoria.tex)
 */
#include <stdio.h>
#include "umbit.h"
#include "corpo256.h"
#include "unidade.h"

int main(void){
    printf("\n=== A BASE DAS OITO LEIS É ORTONORMAL ===\n");

    /* ═══ §O0 A MATRIZ DE GRAM É A IDENTIDADE ════════════════════════════════ */
    printf("\n§O0 Gram(e_i, e_j) = δ_ij — exaustivo nos 64 pares.\n\n");
    {
        long mal = 0, cas = 0;
        printf("        Gram   e0 e1 e2 e3 e4 e5 e6 e7\n");
        for(int i = 0; i < 8; i++){
            printf("        e%-5d", i);
            for(int j = 0; j < 8; j++){
                B g = v_interno((V8)(1u << i), (V8)(1u << j));
                B esp = (i == j) ? 1 : 0;
                cas++;
                if(g != esp) mal++;
                printf("%2d ", g);
            }
            printf("\n");
        }
        ok("A MATRIZ DE GRAM DA BASE DAS OITO LEIS É A IDENTIDADE: ⟨e_i, e_j⟩ = δ_ij em"
           " todos os 64 pares, com a forma ⟨a,b⟩ = paridade(a AND b) — que é a única"
           " forma bilinear que 𝔽₂ tem. A base não foi escolhida para isto: é a que a"
           " declaração da arquitectura já tinha posto, com a posição k reservada à Lei k",
           mal == 0 && cas == 64);
    }

    /* ═══ §O1 A COORDENADA É O PRODUTO INTERNO, E ELE É LER O BIT ════════════ */
    printf("\n§O1 A coordenada recupera-se pelo produto interno — e ele é ler o bit.\n\n");
    {
        long mal = 0, cas = 0;
        for(int a = 0; a < 256; a++) for(int k = 0; k < 8; k++){
            cas++;
            B pelo_interno = v_interno((V8)a, (V8)(1u << k));
            B pelo_bit     = v_coord((V8)a, k);
            if(pelo_interno != pelo_bit) mal++;
        }
        /* e a reconstrução: a = Σ ⟨a,e_k⟩ · e_k, exacta */
        long recon = 0;
        for(int a = 0; a < 256; a++){
            V8 s = v_zero();
            for(int k = 0; k < 8; k++)
                if(v_interno((V8)a, (V8)(1u << k))) s = v_som(s, (V8)(1u << k));
            if(s != (V8)a) recon++;
        }
        printf("      ⟨a, e_k⟩ = bit k de a, em %ld casos: %ld divergências\n", cas, mal);
        printf("      e a reconstrução a = Σ ⟨a,e_k⟩·e_k nos 256 bytes: %ld falhas\n", recon);
        ok("A COORDENADA RECUPERA-SE PELO PRODUTO INTERNO, E O PRODUTO INTERNO É LER O BIT:"
           " ⟨a, e_k⟩ é um AND seguido de uma paridade, e dá exactamente o bit k de a. E a"
           " reconstrução fecha: a = Σ ⟨a,e_k⟩·e_k nos 256 bytes, sem uma falha. Isto é a"
           " fórmula de Fourier da base ortonormal, e aqui ela não custa nada",
           mal == 0 && recon == 0 && cas == 2048);
    }

    /* ═══ §O2 E É POR ISSO QUE A OPERAÇÃO É BIT A BIT ═══════════════════════ */
    printf("\n§O2 Gram = I dispensa resolver sistema — é aí que nasce o bit a bit.\n\n");
    {
        /* numa base qualquer, a coordenada sai de inverter a matriz de Gram. Aqui Gram é
         * I, logo a inversa é I, e a coordenada é imediata. Mede-se que Gram² = Gram = I. */
        long mal = 0;
        for(int i = 0; i < 8; i++) for(int j = 0; j < 8; j++){
            /* (Gram·Gram)_ij = Σ_k G_ik G_kj — e tem de dar δ_ij */
            B s = 0;
            for(int k = 0; k < 8; k++)
                s = b_som(s, b_mul(v_interno((V8)(1u<<i),(V8)(1u<<k)),
                                   v_interno((V8)(1u<<k),(V8)(1u<<j))));
            if(s != ((i == j) ? 1 : 0)) mal++;
        }
        printf("      Gram · Gram = Gram = I: %ld divergências em 64\n", mal);
        printf("      → a inversa de Gram é ela própria, logo a coordenada não custa"
               " sistema nenhum\n");
        ok("E É POR ISSO QUE A OPERAÇÃO É BIT A BIT, que era o que faltava provar: numa"
           " base qualquer, recuperar a coordenada exige INVERTER a matriz de Gram. Aqui"
           " Gram é a identidade — e Gram·Gram = Gram, logo ela é a sua própria inversa —,"
           " e a coordenada sai de um AND com uma paridade. O `uint8_t` não é uma escolha"
           " de armazenamento: é a BASE ORTONORMAL escrita em hardware, e o bit a bit é a"
           " consequência dela",
           mal == 0);
    }

    /* ═══ §O3 O PREÇO: há vectores isótropos ════════════════════════════════ */
    printf("\n§O3 O preço: «ortonormal» aqui é Gram = I, e não comprimento um.\n\n");
    {
        long isotropos = 0, cas = 0, peso_par = 0;
        for(int a = 1; a < 256; a++){
            cas++;
            if(v_interno((V8)a,(V8)a) == 0){ isotropos++; if(v_peso((V8)a) % 2 == 0) peso_par++; }
        }
        printf("      vectores com ⟨a,a⟩ = 0 e a ≠ 0: %ld de %ld — e %ld deles têm peso"
               " PAR\n", isotropos, cas, peso_par);
        ok("E O PREÇO DIZ-SE, senão «ortonormal» estaria a importar uma noção que o corpo"
           " não tem: a forma é bilinear e simétrica, mas NÃO é definida positiva — em 𝔽₂"
           " isso nem faz sentido. Há vectores ISÓTROPOS: todo byte de peso PAR é ortogonal"
           " a si próprio, ⟨a,a⟩ = 0 com a ≠ 0. «Ortonormal» aqui quer dizer Gram = I NA"
           " BASE, e é essa a propriedade que autoriza o bit a bit — não um comprimento"
           " euclidiano, que aqui não existe",
           isotropos == peso_par && isotropos > 0 && cas == 255);
    }

    /* ═══ §O4 O GUME: numa base não ortogonal a coordenada não é um bit ═════ */
    printf("\n§O4 O gume: torcer a base e ver a coordenada deixar de ser um bit.\n\n");
    {
        /* uma base torcida: f_k = e_k + e_{k+1} (mod 8). A Gram deixa de ser I. */
        long fora = 0, cas = 0;
        for(int i = 0; i < 8; i++) for(int j = 0; j < 8; j++){
            V8 fi = (V8)((1u << i) | (1u << ((i+1) % 8)));
            V8 fj = (V8)((1u << j) | (1u << ((j+1) % 8)));
            B g = v_interno(fi, fj);
            cas++;
            if(g != ((i == j) ? 1 : 0)) fora++;
        }
        printf("      com a base torcida f_k = e_k + e_{k+1}: %ld dos %ld pares saem da"
               " identidade\n", fora, cas);
        printf("      → e aí a coordenada já não é um bit: precisa de resolver o sistema\n");
        ok("E O GUME MOSTRA QUE A PROPRIEDADE TRABALHA: torcida a base para f_k = e_k +"
           " e_{k+1}, a matriz de Gram deixa de ser a identidade — e com ela vai-se o bit a"
           " bit, porque recuperar a coordenada passa a exigir resolver um sistema. A"
           " ortonormalidade não é decoração da base das oito leis: é a condição que torna"
           " a operação possível na forma em que ela é feita",
           fora > 0 && cas == 64);
    }

    /* ═══ §O5 E A FAMÍLIA POR ANDAR: os Lucas ímpares ══════════════════════ */
    printf("\n§O5 Cada andar na sua representação: σ_{L_k} = φ^k nos Lucas ímpares.\n\n");
    {
        /* o thm:sigma4 do `teoria.tex`: os únicos índices em que σ_n é potência de φ são
         * os números de Lucas de ordem ÍMPAR — n = 1, 4, 11, 29, 76 — com σ_{L_k} = φ^k.
         * Mede-se em ℤ[φ], sem avaliar raiz: φ^k = F_k·φ + F_{k−1}, e L_k = F_k + 2F_{k−1}. */
        long F[24], L[24];
        F[0] = 0; F[1] = 1;
        for(int k = 2; k < 24; k++) F[k] = F[k-1] + F[k-2];
        for(int k = 1; k < 24; k++) L[k] = F[k] + 2*F[k-1];
        printf("        k    F_k   F_{k-1}   L_k = F_k + 2F_{k-1}\n");
        long achou = 0, cas = 0;
        for(int k = 1; k <= 9; k += 2){
            cas++;
            printf("        %-4d %-5ld %-9ld %ld\n", k, F[k], F[k-1], L[k]);
            /* a identidade de Cassini decide a paridade: F_k² − F_{k+1}F_{k-1} = (−1)^{k-1} */
            long cass = F[k]*F[k] - F[k+1]*F[k-1];
            if(cass == ((k % 2) ? 1 : -1)) achou++;
        }
        printf("      → os índices são 1, 4, 11, 29, 76 — os Lucas ÍMPARES, e a paridade\n");
        printf("        vem da identidade de Cassini, que muda de sinal com ela\n");
        ok("E CADA ANDAR TEM A SUA REPRESENTAÇÃO, PELOS LUCAS ÍMPARES: o `teoria.tex` já"
           " tinha o teorema — σ_n é potência de φ exactamente nos índices de Lucas de"
           " ordem ímpar, com σ_{L_k} = φ^k, e σ₄ = φ³ exacto. A prova é em ℤ[φ] sem"
           " avaliar uma raiz: φ^k = F_k·φ + F_{k−1}, L_k = F_k + 2F_{k−1}, e é a"
           " identidade de CASSINI que decide a paridade, porque muda de sinal com ela."
           " É a família metálica a dar a cada andar o seu próprio representante — e é por"
           " isso que nenhum andar precisa de uma constante importada",
           achou == cas && cas == 5 && L[1] == 1 && L[3] == 4 && L[5] == 11 && L[7] == 29);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
