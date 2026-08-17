/* icecube.c — ISOLA O ASTROFÍSICO no sistema: forward-fold com as IRFs, contagens INTEIRAS,
 * o expoente FIXO pela lei (astro γ=2, a codimensão, p.u.) e só a magnitude a vestir a roupa.
 *
 * Corre sobre os dados públicos de 10 anos (Harvard Dataverse doi:10.7910/DVN/VKL316), em /tmp/icecube.
 * Não usa Python nem floats soltos: as contagens são inteiras e exatas; o modelo é o único sítio
 * onde entra o real, e o expoente dele não é ajustado — é a lei.
 *
 * cc -O2 -std=c99 -w -lm tools/icecube.c -o /tmp/ic && /tmp/ic
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ── grelhas ────────────────────────────────────────────────────────────────────────── */
#define NEV   14        /* E_nu: 2.0..9.0, bins de 0.5 (do smearing)                       */
#define NBAND 3         /* Dec: [-90,-10) [-10,10) [10,90)                                 */
#define ER0   1.0       /* E_reco: de 1.0 a 7.0, bins de 0.5                                */
#define ERW   0.5
#define NER   12
static const double BAND_LO[NBAND]={-90,-10,10}, BAND_HI[NBAND]={-10,10,90};
static int band_of(double d){ for(int b=0;b<NBAND;b++) if(d>=BAND_LO[b]&&d<BAND_HI[b]) return b; return NBAND-1; }
static double sind(double x){ return sin(x*M_PI/180.0); }
static int er_bin(double e){ int i=(int)floor((e-ER0)/ERW); return (i>=0&&i<NER)?i:-1; }
static int ev_bin(double elo){ int i=(int)floor((elo-2.0)/0.5+0.5); return (i>=0&&i<NEV)?i:-1; }

/* ── aceitação: soma A_eff * dΩ sobre os sub-bins (E_nu de 0.2, Dec finos) por (ev,band) ── */
static double accept[NEV][NBAND];   /* cm^2 * sr, integrado no bin de E_nu de 0.5           */
static void le_aeff(const char*f){
    FILE*fp=fopen(f,"r"); if(!fp){ fprintf(stderr,"sem %s\n",f); exit(1);}
    char ln[512];
    while(fgets(ln,sizeof ln,fp)){
        if(ln[0]=='#') continue;
        double e0,e1,d0,d1,A;
        if(sscanf(ln," \"%lf %lf %lf %lf %lf",&e0,&e1,&d0,&d1,&A)!=5) continue;
        int ev=(int)floor(((e0+e1)/2 - 2.0)/0.5);      /* bina pelo CENTRO do bin de 0.2       */
        if(ev<0||ev>=NEV) continue;
        int b=band_of((d0+d1)/2);
        accept[ev][b] += A * fabs(sind(d1)-sind(d0));  /* dΩ do bin fino de Dec                */
    }
    fclose(fp);
}

/* ── smearing: P(E_reco|E_nu,band), marginal em PSF/AngErr, rebin a 0.5 ─────────────────── */
static double P[NEV][NBAND][NER];
static void le_smear(const char*f){
    FILE*fp=fopen(f,"r"); if(!fp){ fprintf(stderr,"sem %s\n",f); exit(1);}
    char ln[512];
    while(fgets(ln,sizeof ln,fp)){
        if(ln[0]=='#') continue;
        double ev0,ev1,d0,d1,er0,er1,a,b_,c,dd,fc;
        if(sscanf(ln," \"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                  &ev0,&ev1,&d0,&d1,&er0,&er1,&a,&b_,&c,&dd,&fc)!=11) continue;
        if(fc<=0) continue;
        int ev=ev_bin(ev0); if(ev<0) continue;
        int b=band_of((d0+d1)/2);
        int er=er_bin((er0+er1)/2); if(er<0) continue;
        P[ev][b][er]+=fc;
    }
    fclose(fp);
}

/* ── contagens INTEIRAS dos eventos, por (E_reco bin, band) ─────────────────────────────── */
static long Nobs[NER];   /* inteiro exato                                                    */
static long Ntot=0;
static void le_eventos(const char*f){
    FILE*fp=fopen(f,"r"); if(!fp) return;
    char ln[512];
    while(fgets(ln,sizeof ln,fp)){
        if(ln[0]=='#') continue;
        double mjd,logE,ae,ra,dec;
        if(sscanf(ln," \"%lf %lf %lf %lf %lf",&mjd,&logE,&ae,&ra,&dec)!=5) continue;
        int er=er_bin(logE); if(er<0) continue;
        Nobs[er]++; Ntot++;
    }
    fclose(fp);
}

/* ── template: T(E_reco) = Σ_{ev,band} E_nu^{-γ}·accept·(E_nu ln10 ΔlogE)·P ─────────────── */
static void template(double gamma, double T[NER]){
    for(int er=0;er<NER;er++) T[er]=0;
    for(int ev=0;ev<NEV;ev++){
        double Enu=pow(10.0, 2.0+0.5*ev+0.25);     /* centro do bin de 0.5                    */
        double flux=pow(Enu,-gamma);
        double jac=Enu*log(10.0)*0.5;              /* dN/dE·dE = dN/dlogE·ΔlogE               */
        for(int b=0;b<NBAND;b++){
            double w=flux*accept[ev][b]*jac;
            if(w<=0) continue;
            for(int er=0;er<NER;er++) T[er]+=w*P[ev][b][er];
        }
    }
}

/* ── ajuste linear não-negativo de 2 componentes: N ≈ a·Tatm + b·Tastro, χ² de Poisson ──── */
static double chi2(double a,double b,double*Tatm,double*Tastro,int lo,int hi){
    double s=0;
    for(int er=lo;er<=hi;er++){
        double pred=a*Tatm[er]+b*Tastro[er];
        if(pred<1e-9)pred=1e-9;
        double o=(double)Nobs[er];
        s+=(o-pred)*(o-pred)/(o>1?o:1);
    }
    return s;
}

int main(int argc,char**argv){
    const char*dir = argc>1?argv[1] : "/tmp/icecube";
    char path[512];
    snprintf(path,sizeof path,"%s/Aeff_IC86_II.tab",dir); le_aeff(path);
    snprintf(path,sizeof path,"%s/smear_IC86_II.tab",dir); le_smear(path);
    snprintf(path,sizeof path,"%s/IC86_II.tab",dir); le_eventos(path);
    printf("eventos IC86_II (inteiro exato): %ld\n\n", Ntot);

    double Tastro[NER]; template(2.0,Tastro);          /* A LEI: expoente 2, sem ajuste        */

    /* atmosférico: deixa o expoente correr (é ruído de fundo, não a lei), só ele; astro fixo 2 */
    int lo=er_bin(2.5), hi=NER-1; while(hi>lo && Nobs[hi]<3) hi--;
    double best=1e18, bg=0,ba=0,bb=0; double Tatm_best[NER];
    for(double g=2.5; g<=4.5; g+=0.05){
        double Tatm[NER]; template(g,Tatm);
        /* grelha em log das magnitudes */
        for(double la=-2; la<=8; la+=0.1){ double a=pow(10,la);
            for(double lb=-4; lb<=6; lb+=0.1){ double b=pow(10,lb);
                double c=chi2(a,b,Tatm,Tastro,lo,hi);
                if(c<best){ best=c; bg=g; ba=a; bb=b; memcpy(Tatm_best,Tatm,sizeof Tatm); }
            }
        }
    }
    int dof=(hi-lo+1)-3;
    printf("AJUSTE (astro γ=2 FIXO pela lei; atm γ e as 2 magnitudes ajustados):\n");
    printf("  γ_atm=%.2f   mag_atm=%.3g   mag_astro=%.3g\n", bg, ba, bb);
    printf("  χ²/dof = %.0f/%d = %.2f\n", best, dof, best/dof);

    /* só-atmosférico (astro=0) para comparar */
    long best0=1e18;
    for(double g=2.5;g<=4.5;g+=0.05){ double Ta[NER]; template(g,Ta);
        for(double la=-2;la<=8;la+=0.05){ double a=pow(10,la); double c=chi2(a,0,Ta,Tastro,lo,hi); if(c<best0)best0=c; } }
    printf("  χ² SÓ atmosférico (astro=0): %.0f   →  o astro melhora Δχ²=%.0f\n\n", best0, best0-best);

    printf("  log10E   N_obs      atm      astro    frac_astro\n");
    for(int er=lo;er<=hi;er++){
        double at=ba*Tatm_best[er], as=bb*Tastro[er], tot=at+as;
        printf("   %4.2f  %8ld  %8.0f %8.0f     %.2f\n",
               ER0+ERW*er+ERW/2, Nobs[er], at, as, tot>0?as/tot:0);
    }
    return 0;
}
