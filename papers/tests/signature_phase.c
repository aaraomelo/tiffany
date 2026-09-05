#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

uint64_t power_mod(uint64_t base, uint64_t exp, uint64_t mod);
uint64_t ord_p(uint64_t x, uint64_t p);

// 1. Assinatura Temporal COMPLETA: G_t(v) para t in [0, L-1], v in F_p^*
typedef struct {
    uint64_t *grid; // Matriz L x p: grid[t * p + v] = G_t(v)
    size_t L;
    size_t p;
} SigZetaComplete;

// 2. Projeção Comprimida A: Histograma Final H_x(v) = G_{L-1}(v)
typedef struct {
    uint64_t *histogram; // Vetor de tamanho p
    size_t p;
} HistogramProjection;

// 3. Projeção Comprimida B: Esqueleto Não-Rotulado (Histograma Ordenado/Multisset)
typedef struct {
    uint64_t *sorted_counts; // Frequências ordenadas (esqueleto combinatório sem a identidade dos elementos)
    size_t p;
} UnlabeledProjection;

// --- Construtores ---

SigZetaComplete compute_sig_complete(uint64_t x, uint64_t p, uint64_t L) {
    SigZetaComplete sig;
    sig.L = L;
    sig.p = p;
    sig.grid = calloc(L * p, sizeof(uint64_t));

    uint64_t curr = 1;
    uint64_t *running_counts = calloc(p, sizeof(uint64_t));

    for (uint64_t t = 0; t < L; t++) {
        running_counts[curr]++;
        for (uint64_t v = 0; v < p; v++) {
            sig.grid[t * p + v] = running_counts[v];
        }
        curr = (curr * x) % p;
    }
    free(running_counts);
    return sig;
}

HistogramProjection compute_histogram_proj(const SigZetaComplete *sig) {
    HistogramProjection proj;
    proj.p = sig->p;
    proj.histogram = malloc(sig->p * sizeof(uint64_t));
    // Copia a última linha da matriz temporal G_{L-1}(v)
    memcpy(proj.histogram, &sig->grid[(sig->L - 1) * sig->p], sig->p * sizeof(uint64_t));
    return proj;
}

int compare_uint64(const void *a, const void *b) {
    uint64_t arg1 = *(const uint64_t *)a;
    uint64_t arg2 = *(const uint64_t *)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

UnlabeledProjection compute_unlabeled_proj(const HistogramProjection *hist) {
    UnlabeledProjection proj;
    proj.p = hist->p;
    proj.sorted_counts = malloc(hist->p * sizeof(uint64_t));
    memcpy(proj.sorted_counts, hist->histogram, hist->p * sizeof(uint64_t));
    // Ordena as frequências para esquecer a identidade dos elementos em F_p^\times
    qsort(proj.sorted_counts, hist->p, sizeof(uint64_t), compare_uint64);
    return proj;
}

// --- Comparadores ---

bool sig_complete_equal(const SigZetaComplete *a, const SigZetaComplete *b) {
    if (a->L != b->L || a->p != b->p) return false;
    return memcmp(a->grid, b->grid, a->L * a->p * sizeof(uint64_t)) == 0;
}

bool hist_proj_equal(const HistogramProjection *a, const HistogramProjection *b) {
    return memcmp(a->histogram, b->histogram, a->p * sizeof(uint64_t)) == 0;
}

bool unlabelled_proj_equal(const UnlabeledProjection *a, const UnlabeledProjection *b) {
    return memcmp(a->sorted_counts, b->sorted_counts, a->p * sizeof(uint64_t)) == 0;
}

// --- Runner de Experimentos ---

void run_rigorous_phase_analysis(uint64_t p, uint64_t L) {
    printf("\n======================================================\n");
    printf(" ANÁLISE DE FIBRAS E COMPRESSÃO (p = %lu, L = %lu)\n", p, L);
    printf("======================================================\n");

    uint64_t num_elements = p - 1;
    SigZetaComplete *sig_comp = malloc(sizeof(SigZetaComplete) * num_elements);
    HistogramProjection *hist_proj = malloc(sizeof(HistogramProjection) * num_elements);
    UnlabeledProjection *unlab_proj = malloc(sizeof(UnlabeledProjection) * num_elements);
    uint64_t *orders = malloc(sizeof(uint64_t) * num_elements);

    for (uint64_t x = 1; x < p; x++) {
        sig_comp[x - 1] = compute_sig_complete(x, p, L);
        hist_proj[x - 1] = compute_histogram_proj(&sig_comp[x - 1]);
        unlab_proj[x - 1] = compute_unlabeled_proj(&hist_proj[x - 1]);
        orders[x - 1] = ord_p(x, p);
    }

    // Medição de N_sig e N_par para cada nível de abstração
    uint64_t n_sig_comp = 0, n_par_comp = 0;
    uint64_t n_sig_hist = 0, n_par_hist = 0;
    uint64_t n_sig_unlab = 0, n_par_unlab = 0;

    for (uint64_t i = 0; i < num_elements; i++) {
        bool new_sig_c = true, new_par_c = true;
        bool new_sig_h = true, new_par_h = true;
        bool new_sig_u = true, new_par_u = true;

        for (uint64_t j = 0; j < i; j++) {
            // Completa
            if (sig_complete_equal(&sig_comp[i], &sig_comp[j])) {
                new_sig_c = false;
                if (orders[i] == orders[j]) new_par_c = false;
            }
            // Histograma
            if (hist_proj_equal(&hist_proj[i], &hist_proj[j])) {
                new_sig_h = false;
                if (orders[i] == orders[j]) new_par_h = false;
            }
            // Não-Rotulado
            if (unlabelled_proj_equal(&unlab_proj[i], &unlab_proj[j])) {
                new_sig_u = false;
                if (orders[i] == orders[j]) new_par_u = false;
            }
        }
        if (new_sig_c) n_sig_comp++; if (new_par_c) n_par_comp++;
        if (new_sig_h) n_sig_hist++; if (new_par_h) n_par_hist++;
        if (new_sig_u) n_sig_unlab++; if (new_par_u) n_par_unlab++;
    }

    printf(" 1. Sig_zeta Completa {G_t(v)}:\n");
    printf("    N_sig = %lu, N_par = %lu -> %s\n", n_sig_comp, n_par_comp, 
           (n_par_comp == n_sig_comp) ? "PRESERVA FASE (Fidelidade Trivial)" : "COLAPSA FASE");

    printf(" 2. Projeção Histograma Final H_x(v):\n");
    printf("    N_sig = %lu, N_par = %lu -> %s\n", n_sig_hist, n_par_hist, 
           (n_par_hist == n_sig_hist) ? "PRESERVA FASE" : "PERDE FASE (N_par > N_sig)");

    printf(" 3. Projeção Não-Rotulada P_iso (Sem ID de Elementos):\n");
    printf("    N_sig = %lu, N_par = %lu -> %s\n", n_sig_unlab, n_par_unlab, 
           (n_par_unlab == n_sig_unlab) ? "PRESERVA FASE" : "PERDE FASE (Colapso Detectado!)");

    // Limpeza
    for (uint64_t i = 0; i < num_elements; i++) {
        free(sig_comp[i].grid);
        free(hist_proj[i].histogram);
        free(unlab_proj[i].sorted_counts);
    }
    free(sig_comp); free(hist_proj); free(unlab_proj); free(orders);
}

int main(void) {
    // Execução em domínios curtos para evitar travamento por tamanho de subgrupo
    run_rigorous_phase_analysis(17, 4);
    run_rigorous_phase_analysis(17, 16);
    run_rigorous_phase_analysis(31, 5);
    return 0;
}