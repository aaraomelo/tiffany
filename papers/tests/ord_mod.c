#include <stdio.h>
#include <stdint.h>
#include <assert.h>

// Exponenciação modular rápida: (base^exp) % mod
uint64_t power_mod(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (uint64_t)((__uint128_t)res * base % mod);
        base = (uint64_t)((__uint128_t)base * base % mod);
        exp /= 2;
    }
    return res;
}

// Inverso modular via Algoritmo de Euclides Estendido
uint64_t inv_mod(uint64_t a, uint64_t m) {
    int64_t m0 = (int64_t)m, t, q;
    int64_t x0 = 0, x1 = 1;
    if (m == 1) return 0;
    int64_t a_signed = (int64_t)a;
    while (a_signed > 1) {
        q = a_signed / m0;
        t = m0; m0 = a_signed % m0; a_signed = t;
        t = x0; x0 = x1 - q * x0; x1 = t;
    }
    if (x1 < 0) x1 += (int64_t)m;
    return (uint64_t)x1;
}

// Calcula ord_p(x) = min { r >= 1 : x^r == 1 (mod p) }
// Requisito: p primo, p > 1, x % p != 0 (Garante x \in F_p^\times)
uint64_t ord_p(uint64_t x, uint64_t p) {
    assert(p > 1 && x % p != 0);
    x %= p;
    if (x == 1) return 1;

    uint64_t n = p - 1;
    uint64_t min_ord = n;

    for (uint64_t d = 1; d * d <= n; d++) {
        if (n % d == 0) {
            if (power_mod(x, d, p) == 1) {
                if (d < min_ord) min_ord = d;
            }
            uint64_t d2 = n / d;
            if (power_mod(x, d2, p) == 1) {
                if (d2 < min_ord) min_ord = d2;
            }
        }
    }
    return min_ord;
}

void test_ord_mod_known_values(void) {
    assert(ord_p(3, 7) == 6);
    assert(ord_p(2, 11) == 10);
    assert(ord_p(7, 19) == 3);
    assert(ord_p(1, 17) == 1);
    assert(ord_p(16, 17) == 2);
    printf("[PASS] ord_mod: Testes unitários de ordem multiplicativa concluídos com sucesso.\n");
}

int main(void) {
    test_ord_mod_known_values();
    return 0;
}