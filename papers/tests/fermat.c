#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

uint64_t ord_p(uint64_t x, uint64_t p);
uint64_t inv_mod(uint64_t a, uint64_t m);

// Fatoração simples para encontrar divisores primos de a
size_t get_prime_factors(uint64_t n, uint64_t *factors) {
    size_t count = 0;
    uint64_t temp = n;
    
    for (uint64_t p = 2; p * p <= temp; p++) {
        if (temp % p == 0) {
            factors[count++] = p;
            while (temp % p == 0) temp /= p;
        }
    }
    if (temp > 1) {
        factors[count++] = temp;
    }
    return count;
}

// Avalia o perfil de fase relacional \rho_p(b,c) = ord_p(b * c^{-1}) para p | a
void analyze_fermat_relational_phase(uint64_t a, uint64_t b, uint64_t c, uint64_t k) {
    printf("\n======================================================\n");
    printf(" PERFIL DE FASE RELACIONAL DE FERMAT: (%lu^%lu + %lu^%lu =? %lu^%lu)\n", a, k, b, k, c, k);
    printf("======================================================\n");

    uint64_t prime_factors[64];
    size_t num_primes = get_prime_factors(a, prime_factors);

    for (size_t i = 0; i < num_primes; i++) {
        uint64_t p = prime_factors[i];
        
        // Verificação de primitividade local: p \nmid b e p \nmid c
        if (b % p == 0 || c % p == 0) {
            printf("[AVISO] Solução não-primitiva localmente para p = %lu\n", p);
            continue;
        }

        uint64_t c_inv = inv_mod(c % p, p);
        uint64_t ratio = (b % p * c_inv) % p;
        uint64_t rho_p = ord_p(ratio, p);

        bool divides_k = (k % rho_p == 0);

        printf(" Primo p | a : %lu\n", p);
        printf("   Ratio (bc^-1 mod p) : %lu\n", ratio);
        printf("   Fase Relacional rho_p(b,c) = ord_p(bc^-1) : %lu\n", rho_p);
        printf("   Condição Local (rho_p | k=%lu) : %s\n", k, divides_k ? "SATISFEITA [OK]" : "VIOLADA [OBSTRUÇÃO]");
    }
}

int main(void) {
    // Teste com tripla pitagórica k=2: 3^2 + 4^2 = 5^2 (a=3, b=4, c=5)
    analyze_fermat_relational_phase(3, 4, 5, 2);

    // Teste sintético de validação de fluxo para k=3: a=6 (primos 2, 3), b=5, c=7
    analyze_fermat_relational_phase(6, 5, 7, 3);

    return 0;
}