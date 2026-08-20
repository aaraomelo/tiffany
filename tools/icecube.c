/* icecube.c — ISOLA O ASTROFÍSICO no sistema: forward-fold com as IRFs, contagens INTEIRAS,
 * o expoente FIXO pela lei (astro γ=2, a codimensão, p.u.) e só a magnitude a vestir a roupa.
 *
 * Corre sobre os dados públicos de 10 anos (Harvard Dataverse doi:10.7910/DVN/VKL316), em /tmp/icecube.
 * Não usa Python nem floats soltos: as contagens são inteiras e exactas; o modelo é inteiro fixo.
 *
 * cc -O2 -std=c99 -w tools/icecube.c -o /tmp/ic && /tmp/ic
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include "../lib/le_emb.h"   /* EMB_S=10⁴ — faixas de declinação ±10° (×1000) */
#include "../lib/reta.h"     /* rt_le_decimal_end — parse exacto na fronteira I/O */

#define NEV   14
#define NBAND 3
#define ER0   1000        /* E_reco ×1000: 1.0..7.0, bins de 0.5 */
#define ERW   500
#define NER   12
#define ICE_SCALE UINT64_C(1000000000) /* 10⁹ — escala dos templates (só fronteira/leitura) */

static const long BAND_LO[NBAND] = {-90000, -EMB_S, EMB_S};   /* declinação ×1000; ±10° = ±EMB_S */
static const long BAND_HI[NBAND] = {-EMB_S, EMB_S, 90000};

static int band_of(long d){
    for(int b = 0; b < NBAND; b++) if(d >= BAND_LO[b] && d < BAND_HI[b]) return b;
    return NBAND - 1;
}
static int er_bin(long e){ int i = (int)((e - ER0) / ERW); return (i >= 0 && i < NER) ? i : -1; }
static int ev_bin(long elo){ int i = (int)((elo - 2000) / 500 + 1); return (i >= 0 && i < NEV) ? i : -1; }

/* sin(deg×1000) × 10⁶ — Taylor em fixo, sem math.h */
static long sin_m6(long deg_milli){
    if(deg_milli > 90000) deg_milli = 90000;
    if(deg_milli < -90000) deg_milli = -90000;
    long neg = 0;
    if(deg_milli < 0){ neg = 1; deg_milli = -deg_milli; }
    /* x rad ×10⁶ ≈ deg_milli * 1745329 / 100 */
    int64_t x = (int64_t)deg_milli * 1745329 / 100;
    int64_t x3 = x * x / 1000000 * x / 1000000;
    int64_t x5 = x3 * x / 1000000 * x / 1000000;
    int64_t s = x - x3 / 6 + x5 / 120;
    if(neg) s = -s;
    return (long)s;
}

static int64_t parse_fp(const char **pp, int64_t out_scale){
    const char *p = *pp;
    while(*p == ' ' || *p == '\t' || *p == '"') p++;
    int sg; long num, den; const char *e;
    if(!rt_le_decimal_end(p, &sg, &num, &den, &e)) return 0;
    p = e;
    while(*p == ' ' || *p == '\t' || *p == '"') p++;
    *pp = p;
    int64_t n = (int64_t)num * out_scale + den / 2;
    int64_t v = n / den;
    return sg < 0 ? -v : v;
}

static uint64_t pow10_scaled(int num, int den, int out_scale){
    /* 10^(num/den) × out_scale — num/den em décimos de unidade log10 */
    uint64_t r = (uint64_t)out_scale;
    int e = num / den;
    int f = num % den;
    if(e >= 0) for(int i = 0; i < e; i++) r *= 10;
    else for(int i = 0; i < -e; i++) r = (r + 5) / 10;
    if(f){
        static const uint64_t tenth[10] = {1000000000,1258925411,1584893192,1995262314,2511886431,
                                           3162277660,3981071705,5011872336,6309573444,7943282347};
        r = r * tenth[f] / ICE_SCALE;
    }
    return r;
}

static long accept[NEV][NBAND];
static void le_aeff(const char *f){
    FILE *fp = fopen(f, "r");
    if(!fp){ fprintf(stderr, "sem %s\n", f); exit(1); }
    char ln[512];
    while(fgets(ln, sizeof ln, fp)){
        if(ln[0] == '#') continue;
        const char *p = ln;
        int64_t e0 = parse_fp(&p, 1000);
        int64_t e1 = parse_fp(&p, 1000);
        int64_t d0 = parse_fp(&p, 1000);
        int64_t d1 = parse_fp(&p, 1000);
        int64_t A  = parse_fp(&p, (int)(ICE_SCALE / 1000));
        int ev = (int)(((e0 + e1) / 2 - 2000) / 500);
        if(ev < 0 || ev >= NEV) continue;
        int b = band_of((int)((d0 + d1) / 2));
        long ds = sin_m6((int)d1) - sin_m6((int)d0);
        if(ds < 0) ds = -ds;
        accept[ev][b] += (long)(A * ds / 1000000);
    }
    fclose(fp);
}

static long P[NEV][NBAND][NER];
static void le_smear(const char *f){
    FILE *fp = fopen(f, "r");
    if(!fp){ fprintf(stderr, "sem %s\n", f); exit(1); }
    char ln[512];
    while(fgets(ln, sizeof ln, fp)){
        if(ln[0] == '#') continue;
        const char *p = ln;
        int64_t ev0 = parse_fp(&p, 1000);
        (void)parse_fp(&p, 1000);
        int64_t d0 = parse_fp(&p, 1000);
        int64_t d1 = parse_fp(&p, 1000);
        int64_t er0 = parse_fp(&p, 1000);
        (void)parse_fp(&p, 1000);
        for(int k = 0; k < 5; k++) (void)parse_fp(&p, 1000);
        int64_t fc = parse_fp(&p, (int)ICE_SCALE);
        if(fc <= 0) continue;
        int ev = ev_bin((int)ev0); if(ev < 0) continue;
        int b = band_of((int)((d0 + d1) / 2));
        int er = er_bin((int)er0); if(er < 0) continue;
        P[ev][b][er] += (long)(fc / (int64_t)ICE_SCALE);
    }
    fclose(fp);
}

static long Nobs[NER];
static long Ntot = 0;
static void le_eventos(const char *f){
    FILE *fp = fopen(f, "r");
    if(!fp) return;
    char ln[512];
    while(fgets(ln, sizeof ln, fp)){
        if(ln[0] == '#') continue;
        const char *p = ln;
        (void)parse_fp(&p, 1000);
        int64_t logE = parse_fp(&p, 1000);
        (void)parse_fp(&p, 1000);
        (void)parse_fp(&p, 1000);
        (void)parse_fp(&p, 1000);
        int er = er_bin((int)logE); if(er < 0) continue;
        Nobs[er]++; Ntot++;
    }
    fclose(fp);
}

static uint64_t Enu_sc[NEV];

static void init_enu(void){
    for(int ev = 0; ev < NEV; ev++)
        Enu_sc[ev] = pow10_scaled(9 + 2 * ev, 4, (int)ICE_SCALE);
}

static uint64_t isqrt_u64(uint64_t n){
    if(n == 0) return 0;
    uint64_t x = n, y = (x + 1) >> 1;
    while(y < x){ x = y; y = (x + n / x) >> 1; }
    return x;
}

/* E^exp × ICE_SCALE com E já × ICE_SCALE; exp = num/den (den>0) */
static uint64_t epow(uint64_t E, int num, int den){
    if(num == 0) return ICE_SCALE;
    if(num > 0){
        uint64_t r = ICE_SCALE;
        for(int i = 0; i < num; i++) r = r * E / ICE_SCALE;
        if(den == 1) return r;
        for(int i = 0; i < den - 1; i++) r = isqrt_u64(r * ICE_SCALE);
        return r;
    }
    uint64_t r = ICE_SCALE;
    for(int i = 0; i < -num; i++) r = r * ICE_SCALE / E;
    if(den == 1) return r;
    for(int i = 0; i < den - 1; i++) r = isqrt_u64(r * ICE_SCALE);
    return r;
}

static void template_gamma(int gamma_milli, uint64_t T[NER]){
    for(int er = 0; er < NER; er++) T[er] = 0;
    for(int ev = 0; ev < NEV; ev++){
        /* flux·jac = Enu^(1−γ) · (ln10·0.5) */
        uint64_t fj = epow(Enu_sc[ev], 1000 - gamma_milli, 1000);
        fj = fj * 217434u / 1000000u;
        for(int b = 0; b < NBAND; b++){
            uint64_t w = fj * (uint64_t)accept[ev][b] / ICE_SCALE;
            if(w == 0) continue;
            for(int er = 0; er < NER; er++) T[er] += w * (uint64_t)P[ev][b][er] / ICE_SCALE;
        }
    }
}

static uint64_t chi2_fit(uint64_t a, uint64_t b, uint64_t *Tatm, uint64_t *Tastro, int lo, int hi){
    uint64_t s = 0;
    for(int er = lo; er <= hi; er++){
        uint64_t pred = a * Tatm[er] / ICE_SCALE + b * Tastro[er] / ICE_SCALE;
        if(pred < 1) pred = 1;
        int64_t o = Nobs[er];
        int64_t d = o - (int64_t)pred;
        uint64_t denom = o > 1 ? (uint64_t)o : 1u;
        s += (uint64_t)(d * d) / denom;
    }
    return s;
}

static uint64_t mag_tab[121];   /* 10^(la/10) × ICE_SCALE, la = -40..80 */
static void init_mag(void){
    for(int la = -40; la <= 80; la++)
        mag_tab[la + 40] = pow10_scaled(la, 10, (int)ICE_SCALE);
}

int main(int argc, char **argv){
    init_mag();
    init_enu();
    const char *dir = argc > 1 ? argv[1] : "/tmp/icecube";
    char path[512];
    snprintf(path, sizeof path, "%s/Aeff_IC86_II.tab", dir); le_aeff(path);
    snprintf(path, sizeof path, "%s/smear_IC86_II.tab", dir); le_smear(path);
    snprintf(path, sizeof path, "%s/IC86_II.tab", dir); le_eventos(path);
    printf("eventos IC86_II (inteiro exacto): %ld\n\n", Ntot);

    uint64_t Tastro[NER];
    template_gamma(2000, Tastro);

    int lo = er_bin(2500), hi = NER - 1;
    while(hi > lo && Nobs[hi] < 3) hi--;
    uint64_t best = UINT64_MAX, ba = 0, bb = 0;
    long bg = 0;
    uint64_t Tatm_best[NER];
    for(int g = 2500; g <= 4500; g += 50){
        uint64_t Tatm[NER];
        template_gamma(g, Tatm);
        for(int la = -20; la <= 80; la++){
            uint64_t a = mag_tab[la + 40];
            for(int lb = -40; lb <= 60; lb++){
                uint64_t b = mag_tab[lb + 40];
                uint64_t c = chi2_fit(a, b, Tatm, Tastro, lo, hi);
                if(c < best){
                    best = c; bg = g; ba = a; bb = b;
                    for(int i = 0; i < NER; i++) Tatm_best[i] = Tatm[i];
                }
            }
        }
    }
    int dof = (hi - lo + 1) - 3;
    printf("AJUSTE (astro γ=2 FIXO pela lei; atm γ e as 2 magnitudes ajustados):\n");
    printf("  γ_atm=%ld.%02ld   mag_atm=%" PRIu64 "   mag_astro=%" PRIu64 "\n",
           bg / 1000, (bg % 1000) / 10, ba, bb);
    printf("  χ²/dof = %" PRIu64 "/%d = %" PRIu64 "\n", best, dof, dof ? best / (uint64_t)dof : best);

    uint64_t best0 = UINT64_MAX;
    for(int g = 2500; g <= 4500; g += 50){
        uint64_t Ta[NER];
        template_gamma(g, Ta);
        for(int la = -20; la <= 80; la++){
            uint64_t a = mag_tab[la + 40];
            uint64_t c = chi2_fit(a, 0, Ta, Tastro, lo, hi);
            if(c < best0) best0 = c;
        }
    }
    printf("  χ² SÓ atmosférico (astro=0): %" PRIu64 "   →  o astro melhora Δχ²=%" PRIu64 "\n\n",
           best0, best0 - best);

    printf("  log10E   N_obs      atm      astro    frac_astro\n");
    for(int er = lo; er <= hi; er++){
        uint64_t at = ba * Tatm_best[er] / ICE_SCALE;
        uint64_t as = bb * Tastro[er] / ICE_SCALE;
        uint64_t tot = at + as;
        long frac = tot > 0 ? (long)(as * 100 / tot) : 0;
        printf("   %d.%03d  %8ld  %8" PRIu64 " %8" PRIu64 "     %ld.%02ld\n",
               (ER0 + ERW * er + ERW / 2) / 1000,
               (ER0 + ERW * er + ERW / 2) % 1000,
               Nobs[er], at, as, frac / 100, frac % 100);
    }
    return 0;
}
