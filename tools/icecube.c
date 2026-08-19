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

#define NEV   14
#define NBAND 3
#define ER0   1000        /* E_reco ×1000: 1.0..7.0, bins de 0.5 */
#define ERW   500
#define NER   12
#define SCALE 1000000000LL /* 10⁹ — escala interna dos templates */

static const long BAND_LO[NBAND] = {-90000, -10000, 10000};   /* declinação ×1000 */
static const long BAND_HI[NBAND] = {-10000, 10000, 90000};

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
    long long x = (long long)deg_milli * 1745329LL / 100LL;
    long long x3 = x * x / 1000000LL * x / 1000000LL;
    long long x5 = x3 * x / 1000000LL * x / 1000000LL;
    long long s = x - x3 / 6LL + x5 / 120LL;
    if(neg) s = -s;
    return (long)s;
}

static long long parse_fp(const char **pp, int out_scale){
    const char *p = *pp;
    while(*p == ' ' || *p == '\t' || *p == '"') p++;
    int neg = 0;
    if(*p == '-'){ neg = 1; p++; }
    long long v = 0;
    while(isdigit((unsigned char)*p)) v = v * 10 + (*p++ - '0');
    int sc = out_scale;
    if(*p == '.'){
        p++;
        while(isdigit((unsigned char)*p) && sc > 1){
            sc /= 10;
            v = v * 10 + (*p++ - '0');
        }
        while(isdigit((unsigned char)*p)) p++;
    }
    while(*p == ' ' || *p == '\t') p++;
    int es = 0;
    if(*p == 'e' || *p == 'E'){
        p++;
        int en = 0;
        if(*p == '-'){ es = 1; p++; }
        else if(*p == '+') p++;
        while(isdigit((unsigned char)*p)) en = en * 10 + (*p++ - '0');
        if(es) for(int i = 0; i < en; i++) sc *= 10;
        else   for(int i = 0; i < en; i++) sc = (sc + 9) / 10;
    }
    while(*p == ' ' || *p == '\t' || *p == '"') p++;
    *pp = p;
    long long r = v * (out_scale / sc);
    return neg ? -r : r;
}

static long long pow10_scaled(int num, int den, int out_scale){
    /* 10^(num/den) × out_scale — num/den em décimos de unidade log10 */
    long long r = out_scale;
    int e = num / den;
    int f = num % den;
    if(e >= 0) for(int i = 0; i < e; i++) r *= 10;
    else for(int i = 0; i < -e; i++) r = (r + 5) / 10;
    if(f){
        static const long tenth[10] = {1000000000,1258925411,1584893192,1995262314,2511886431,
                                       3162277660,3981071705,5011872336,6309573444,7943282347};
        r = r * tenth[f] / SCALE;
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
        long long e0 = parse_fp(&p, 1000);
        long long e1 = parse_fp(&p, 1000);
        long long d0 = parse_fp(&p, 1000);
        long long d1 = parse_fp(&p, 1000);
        long long A  = parse_fp(&p, SCALE / 1000);
        int ev = (int)(((e0 + e1) / 2 - 2000) / 500);
        if(ev < 0 || ev >= NEV) continue;
        int b = band_of((int)((d0 + d1) / 2));
        long ds = sin_m6((int)d1) - sin_m6((int)d0);
        if(ds < 0) ds = -ds;
        accept[ev][b] += (long)(A * ds / 1000000LL);
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
        long long ev0 = parse_fp(&p, 1000);
        (void)parse_fp(&p, 1000);
        long long d0 = parse_fp(&p, 1000);
        long long d1 = parse_fp(&p, 1000);
        long long er0 = parse_fp(&p, 1000);
        (void)parse_fp(&p, 1000);
        for(int k = 0; k < 5; k++) (void)parse_fp(&p, 1000);
        long long fc = parse_fp(&p, SCALE);
        if(fc <= 0) continue;
        int ev = ev_bin((int)ev0); if(ev < 0) continue;
        int b = band_of((int)((d0 + d1) / 2));
        int er = er_bin((int)er0); if(er < 0) continue;
        P[ev][b][er] += (long)(fc / SCALE);
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
        long long logE = parse_fp(&p, 1000);
        (void)parse_fp(&p, 1000);
        (void)parse_fp(&p, 1000);
        (void)parse_fp(&p, 1000);
        int er = er_bin((int)logE); if(er < 0) continue;
        Nobs[er]++; Ntot++;
    }
    fclose(fp);
}

static long long Enu_sc[NEV];

static void init_enu(void){
    for(int ev = 0; ev < NEV; ev++)
        Enu_sc[ev] = pow10_scaled(9 + 2 * ev, 4, SCALE);
}

static long long isqrt128(long long n){
    if(n <= 0) return 0;
    long long x = n, y = (x + 1) >> 1;
    while(y < x){ x = y; y = (x + n / x) >> 1; }
    return x;
}

/* E^exp × SCALE com E já × SCALE; exp = num/den (den>0) */
static long long epow(long long E, int num, int den){
    if(num == 0) return SCALE;
    if(num > 0){
        long long r = SCALE;
        for(int i = 0; i < num; i++) r = r * E / SCALE;
        if(den == 1) return r;
        for(int i = 0; i < den - 1; i++) r = isqrt128(r * SCALE);
        return r;
    }
    long long r = SCALE;
    for(int i = 0; i < -num; i++) r = r * SCALE / E;
    if(den == 1) return r;
    for(int i = 0; i < den - 1; i++) r = isqrt128(r * SCALE);
    return r;
}

static void template_gamma(int gamma_milli, long long T[NER]){
    for(int er = 0; er < NER; er++) T[er] = 0;
    for(int ev = 0; ev < NEV; ev++){
        /* flux·jac = Enu^(1−γ) · (ln10·0.5) */
        long long fj = epow(Enu_sc[ev], 1000 - gamma_milli, 1000);
        fj = fj * 217434LL / 1000000LL;
        for(int b = 0; b < NBAND; b++){
            long long w = fj * accept[ev][b] / SCALE;
            if(w <= 0) continue;
            for(int er = 0; er < NER; er++) T[er] += w * P[ev][b][er] / SCALE;
        }
    }
}

static long long chi2_ll(long long a, long long b, long long *Tatm, long long *Tastro, int lo, int hi){
    long long s = 0;
    for(int er = lo; er <= hi; er++){
        long long pred = a * Tatm[er] / SCALE + b * Tastro[er] / SCALE;
        if(pred < 1) pred = 1;
        long long o = Nobs[er];
        long long d = o - pred;
        long long denom = o > 1 ? o : 1;
        s += d * d / denom;
    }
    return s;
}

static long long mag_tab[121];   /* 10^(la/10) × SCALE, la = -40..80 */
static void init_mag(void){
    for(int la = -40; la <= 80; la++)
        mag_tab[la + 40] = pow10_scaled(la, 10, SCALE);
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

    long long Tastro[NER];
    template_gamma(2000, Tastro);

    int lo = er_bin(2500), hi = NER - 1;
    while(hi > lo && Nobs[hi] < 3) hi--;
    long long best = 9223372036854775807LL, bg = 0, ba = 0, bb = 0;
    long long Tatm_best[NER];
    for(int g = 2500; g <= 4500; g += 50){
        long long Tatm[NER];
        template_gamma(g, Tatm);
        for(int la = -20; la <= 80; la++){
            long long a = mag_tab[la + 40];
            for(int lb = -40; lb <= 60; lb++){
                long long b = mag_tab[lb + 40];
                long long c = chi2_ll(a, b, Tatm, Tastro, lo, hi);
                if(c < best){
                    best = c; bg = g; ba = a; bb = b;
                    for(int i = 0; i < NER; i++) Tatm_best[i] = Tatm[i];
                }
            }
        }
    }
    int dof = (hi - lo + 1) - 3;
    printf("AJUSTE (astro γ=2 FIXO pela lei; atm γ e as 2 magnitudes ajustados):\n");
    printf("  γ_atm=%ld.%02ld   mag_atm=%lld   mag_astro=%lld\n",
           bg / 1000, (bg % 1000) / 10, ba, bb);
    printf("  χ²/dof = %lld/%d = %lld\n", best, dof, dof ? best / dof : best);

    long long best0 = 9223372036854775807LL;
    for(int g = 2500; g <= 4500; g += 50){
        long long Ta[NER];
        template_gamma(g, Ta);
        for(int la = -20; la <= 80; la++){
            long long a = mag_tab[la + 40];
            long long c = chi2_ll(a, 0, Ta, Tastro, lo, hi);
            if(c < best0) best0 = c;
        }
    }
    printf("  χ² SÓ atmosférico (astro=0): %lld   →  o astro melhora Δχ²=%lld\n\n",
           best0, best0 - best);

    printf("  log10E   N_obs      atm      astro    frac_astro\n");
    for(int er = lo; er <= hi; er++){
        long long at = ba * Tatm_best[er] / SCALE;
        long long as = bb * Tastro[er] / SCALE;
        long long tot = at + as;
        long frac = tot > 0 ? as * 100 / tot : 0;
        printf("   %ld.%03ld  %8ld  %8lld %8lld     %ld.%02ld\n",
               (ER0 + ERW * er + ERW / 2) / 1000,
               (ER0 + ERW * er + ERW / 2) % 1000,
               Nobs[er], at, as, frac / 100, frac % 100);
    }
    return 0;
}
