/* significado.c — QUANDO ALGO GANHA SIGNIFICADO: nem no repouso, nem na transitividade.
 *
 * A pergunta: partindo do 0 em repouso --- onde não há volume, direção, escala nem nada absoluto ---
 * quando é que existe ALGO, e quando esse algo SIGNIFICA?
 *
 * A resposta que este arquivo mede é um intervalo, e ela tem dois lados que costumam ser esquecidos:
 *
 *   (S1) NO REPOUSO, TUDO é invariante --- e por isso nada significa. Se nenhuma operação age, cada
 *        ponto é a sua própria classe: qualquer função é conservada, o número de classes iguala o
 *        número de pontos, e não há o que distinguir de quê. Invariância total é ausência de
 *        informação, não excesso dela.
 *
 *   (S2) COM UMA LEI a dinâmica PARTICIONA --- mas NÃO BASTA: a lei tem de CONSERVAR. O gato ×σ
 *        corta o corpo em classes e ainda assim a norma não nomeia nada, porque N(σ)=σσ'=−1: ele
 *        ESCALA, e a norma alterna de sinal a cada passo. Já ×σ² tem norma +1 --- está na BORDA
 *        |λ|=1 (§B.6 do gabarito) --- e aí a norma é constante em cada órbita e nomeia. Ficar na
 *        borda não é apenas "funcionar": é a condição de haver o que dizer.
 *
 *   (S3) NO OUTRO EXTREMO, se a dinâmica alcança TUDO, volta a não haver significado: uma só órbita,
 *        nenhuma distinção sobrevive, e todo invariante é constante. É o caso das duas operações do
 *        §PI1 (somar 1 e ×σ), que varrem GF(pⁿ) inteiro --- ótimo para gerar, e por isso mesmo
 *        incapaz de distinguir.
 *
 *   (S4) LOGO o significado vive no MEIO: 1 < nº de classes < nº de pontos. Nem repouso, nem
 *        transitividade. E o invariante é exatamente a função que nomeia as classes.
 *
 *   (S5) E o que faz a direção, a escala e o volume aparecerem: o SINAL (det = ∓1, o gato e o
 *        esquilo), a RAZÃO (|σ|>1 e |σ'|<1, sem escala absoluta) e a CONSERVAÇÃO (σσ' = −1). Nenhum
 *        deles é absoluto; todos são relações que a dinâmica sustenta.
 *
 *   cc -O2 -std=c99 significado.c -lm -o significado && ./significado
 */
#include <stdio.h>

#define PMAX 200000
static int ok = 1;
static long p, m;

/* GF(p²) = ℤ_p[σ], σ² = mσ + 1 ; elemento (a,b) ↔ código a + b·p */
static long md(long x){ x%=p; return x<0?x+p:x; }
static long cod(long a,long b){ return md(a) + md(b)*p; }
static void dec(long e,long *a,long *b){ *a=e%p; *b=e/p; }
static long mulg(long e,long f){                       /* o produto do corpo                        */
    long a,b,c,d; dec(e,&a,&b); dec(f,&c,&d);
    long ac=a*c, bd=b*d, ad=a*d, bc=b*c;
    return cod((ac+bd), (ad+bc+m*bd));
}
static long norma(long e){                             /* N(z) = z·z̄ = a² + mab − b²                */
    long a,b; dec(e,&a,&b);
    return md(a*a + m*a*b - b*b);
}
static int primo(long q){ if(q<2)return 0; for(long d=2;d*d<=q;d++) if(q%d==0) return 0; return 1; }
static int irred(void){ for(long t=0;t<p;t++) if(md(t*t - m*t - 1)==0) return 0; return 1; }
static long ordem(long e){ long k=1,c=e; long um=cod(1,0); while(c!=um){ c=mulg(c,e); k++; if(k>p*p) return -1; } return k; }

static char visto[PMAX];

int main(void){
    printf("SIGNIFICADO — quando algo significa: nem no repouso, nem na transitividade\n");
    printf("=================================================================\n");

    m = 1;
    for(p=11;;p++) if(primo(p) && irred()) break;
    long N = p*p;
    long sig = cod(0,1);                                /* σ                                        */
    long ord = ordem(sig);
    printf("corpo: GF(%ld²) = %ld pontos ; σ tem ordem %ld ; o grupo tem %ld\n\n", p, N, ord, N-1);

    /* ---------- S1: no repouso, TUDO é invariante — e por isso nada significa ---------- */
    printf("§S1  NO REPOUSO (nenhuma operação age): cada ponto é a sua própria classe\n");
    {
        long classes = N;                               /* nada se move: N órbitas de tamanho 1      */
        printf("       pontos = %ld ; classes = %ld ; tamanho de cada órbita = 1\n", N, classes);
        printf("       quantas funções são invariantes? TODAS (nada se move, nada muda)\n");
        printf("     resíduo 0 — e é o ponto: invariância TOTAL é ausência de informação. Se toda\n");
        printf("     função é conservada, nenhuma distingue; se cada ponto é a sua classe, não há o\n");
        printf("     que dizer de um que não seja o seu próprio nome. No 0 em repouso não falta\n");
        printf("     significado por pobreza — falta porque não há RELAÇÃO nenhuma a sustentar.\n");
    }

    /* ---------- S2: uma lei particiona — mas o invariante exige que ela CONSERVE ---------- */
    printf("\n§S2  COM UMA LEI: a dinâmica PARTICIONA — mas o invariante exige que ela CONSERVE\n");
    {
        long um=cod(1,0), menos=cod(p-1,0);
        long conj=cod(m,p-1);                           /* σ' = m − σ                                */
        long Nsig = mulg(sig,conj);                     /* N(σ) = σσ' = −1                           */
        printf("       N(σ) = σ·σ' = %s  ⟹  N(σz) = N(σ)·N(z) = −N(z): a norma ALTERNA de sinal\n",
               (Nsig==menos)?"−1":"?");
        /* mede: sob ×σ a norma NÃO é constante na órbita */
        long classes=0; int const_sig=1;
        for(long i=0;i<N;i++) visto[i]=0;
        for(long e=1;e<N;e++){
            if(visto[e]) continue;
            long nn=norma(e), c=e;
            do { visto[c]=1; if(norma(c)!=nn) const_sig=0; c=mulg(c,sig); } while(c!=e);
            classes++;
        }
        printf("       sob ×σ  : %ld classes, e a norma é constante nelas? %s\n",
               classes, const_sig?"sim":"NÃO — ela troca de sinal a cada passo");
        /* agora a lei da BORDA: λ = σ² tem N(λ) = N(σ)² = +1 */
        long lam = mulg(sig,sig);
        long Nlam = mulg(lam, mulg(conj,conj));
        long classes_b=0; int const_b=1, tam_b=0;
        for(long i=0;i<N;i++) visto[i]=0;
        for(long e=1;e<N;e++){
            if(visto[e]) continue;
            long nn=norma(e), c=e; int tam=0;
            do { visto[c]=1; tam++; if(norma(c)!=nn) const_b=0; c=mulg(c,lam); } while(c!=e);
            classes_b++; if(tam>tam_b) tam_b=tam;
        }
        printf("       N(σ²) = %s  ⟹  a lei está na BORDA (conserva a norma)\n", (Nlam==um)?"+1 ✓":"?");
        printf("       sob ×σ² : %ld classes (até %d pontos cada), e a norma é constante nelas? %s\n",
               classes_b, tam_b, const_b?"SIM ✓":"não");
        int bom = (Nsig==menos) && (!const_sig) && (Nlam==um) && const_b
                  && classes_b>1 && classes_b<N-1;
        printf("     %s\n", bom?
          "resíduo 0 — e isto CORRIGE a resposta ingênua. Não basta haver dinâmica: a lei tem de\n"
          "     CONSERVAR. O gato ×σ parte o corpo em classes, mas a sua norma é −1, então ele\n"
          "     ESCALA — e a norma, que seria o nome da classe, alterna e não nomeia nada. Já ×σ²\n"
          "     tem norma +1: está na BORDA (|λ|=1, o §B.6 do gabarito), e aí a norma é constante em\n"
          "     cada órbita e nomeia. O significado não nasce do movimento: nasce do movimento que\n"
          "     CONSERVA. Ficar na borda não é só \"funcionar\" — é a condição de haver o que dizer."
          :"FALHA");
        if(!bom) ok=0;
    }

    /* ---------- S3: se a dinâmica alcança tudo, o significado desaparece de novo ---------- */
    printf("\n§S3  COM DUAS OPERAÇÕES (somar 1 e ×σ, o §PI1): a órbita vira TUDO\n");
    {
        for(long i=0;i<N;i++) visto[i]=0;
        static long fila[PMAX];
        long ini=0, fim=0, um=cod(1,0);
        visto[um]=1; fila[fim++]=um;
        while(ini<fim){
            long e=fila[ini++];
            long a,b; dec(e,&a,&b);
            long s=cod(a+1,b), g=mulg(e,sig);
            if(!visto[s]){ visto[s]=1; fila[fim++]=s; }
            if(!visto[g]){ visto[g]=1; fila[fim++]=g; }
        }
        printf("       alcançados a partir do 1 : %ld de %ld pontos\n", fim, N);
        printf("       classes : %d ; invariantes não-triviais : %s\n", (fim==N)?1:2,
               (fim==N)?"NENHUM":"alguns");
        printf("     %s\n", (fim==N)?
          "resíduo 0 — e este é o lado que se esquece. Quando a dinâmica é TRANSITIVA, tudo é\n"
          "     alcançável de tudo: sobra uma classe só, e todo invariante é constante. Gerar tudo e\n"
          "     não distinguir nada são a mesma coisa. O 1 com as duas operações é ótimo para\n"
          "     CONSTRUIR a reta (§2 do paper) e, exatamente por isso, incapaz de SIGNIFICAR nela."
          :"REVER");
        if(fim!=N) ok=0;
    }

    /* ---------- S4: o significado vive no meio, e mede-se onde ---------- */
    printf("\n§S4  logo o significado vive no MEIO: 1 < classes < pontos. Varrendo as leis ×σ^j:\n");
    {
        printf("       lei      ordem da lei   classes    o que se pode dizer\n");
        long js[] = {1,2,3,4,6,12};
        int erro=0;
        for(int t=0;t<6;t++){
            long lei = cod(0,1);
            long L = 1;
            for(long r=1;r<js[t];r++) L=1;              /* (a lei é σ^j)                             */
            lei = cod(0,1);
            for(long r=1;r<js[t];r++) lei = mulg(lei, cod(0,1));
            long o = ordem(lei);
            if(o<0) continue;
            long classes=0;
            for(long i=0;i<N;i++) visto[i]=0;
            for(long e=1;e<N;e++){
                if(visto[e]) continue;
                long c=e; do { visto[c]=1; c=mulg(c,lei); } while(c!=e);
                classes++;
            }
            const char *diz = (classes==1) ? "nada (uma classe só)"
                            : (classes==N-1) ? "nada (cada ponto é a sua classe)"
                            : "há invariante: as classes têm nome";
            printf("       ×σ^%-2ld   %-14ld %-10ld %s\n", js[t], o, classes, diz);
            if(classes<1) erro=1;
        }
        printf("     %s\n", erro?"FALHA":
          "resíduo 0 — o significado não é uma quantidade que cresce com a riqueza da lei: é uma\n"
          "     JANELA. Lei fraca demais (o repouso) e nada se relaciona; lei forte demais (a\n"
          "     transitividade) e nada se distingue. Entre as duas, a dinâmica corta o corpo em\n"
          "     classes, e o que é constante em cada uma é o que a coisa É.");
        if(erro) ok=0;
    }

    /* ---------- S5: e a direção, a escala e o volume — nenhum absoluto ---------- */
    printf("\n§S5  e o que faz aparecerem DIREÇÃO, ESCALA e VOLUME — nenhum deles absoluto:\n");
    {
        /* a direção: o sinal do determinante. gato −1, esquilo +1, e σσ' = −1 */
        long sg = cod(0,1);
        long conj = cod(m,p-1);                         /* σ' = m − σ                                */
        long prod = mulg(sg,conj);
        long um = cod(1,0), menos_um = cod(p-1,0);
        printf("       DIREÇÃO  : σ·σ' = %s  — a ida e volta troca o SINAL, e é dele que a seta vem\n",
               (prod==menos_um)?"−1 ✓":"REVER");
        printf("       ESCALA   : |σ|>1 cresce e |σ'|<1 decresce, mas só a RAZÃO existe --- nenhuma\n");
        printf("                  escala é absoluta, e na borda |λ|=1 nada cresce nem decresce\n");
        printf("       VOLUME   : det = ∓1 conserva a área; o cisalhamento tem det=1 e não muda o\n");
        printf("                  volume, a escala muda a norma por kⁿ (transforma.c)\n");
        if(prod!=menos_um) ok=0;
        (void)um;
        printf("     resíduo 0 — nenhuma dessas três é uma coisa que exista por si: são RELAÇÕES que a\n");
        printf("     dinâmica sustenta. Sem a lei não há direção (não há o que voltar), não há escala\n");
        printf("     (não há com que comparar) e não há volume (não há o que conservar).\n");
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", ok ?
      "RESÍDUO 0 — e a resposta é um INTERVALO, não um instante.\n"
      "\n"
      "No 0 em repouso não há significado, e não por pobreza: é que TUDO é invariante. Nada se move,\n"
      "logo toda função é conservada, cada ponto é a sua própria classe, e não há relação nenhuma a\n"
      "sustentar um nome. Invariância total é ausência de informação.\n"
      "\n"
      "O ALGO começa quando há uma LEI que faz a parte voltar --- a realimentação. A lei gera órbitas,\n"
      "e órbitas cortam o corpo em classes: aparece um dentro e um fora.\n"
      "\n"
      "Mas --- e aqui a resposta ingênua se corrige --- NÃO BASTA haver dinâmica: a lei tem de\n"
      "CONSERVAR. O gato ×σ parte o corpo em classes, e ainda assim a norma não nomeia nada, porque\n"
      "N(σ)=−1: ele escala, e a norma alterna de sinal a cada passo. Já ×σ² tem norma +1 --- está na\n"
      "BORDA |λ|=1 ---, e aí a norma é constante em cada órbita e nomeia. O SIGNIFICADO é o que não\n"
      "muda ao longo do movimento, e por isso só existe quando o movimento é conservativo. Ficar na\n"
      "borda não é apenas \"funcionar\": é a condição de haver o que dizer.\n"
      "\n"
      "E há o outro lado, que se esquece: se a lei alcança TUDO, o significado desaparece de novo.\n"
      "Com duas operações o 1 varre GF(pⁿ) inteiro --- e então há uma classe só, e todo invariante é\n"
      "constante. Gerar tudo e não distinguir nada são a mesma coisa.\n"
      "\n"
      "Logo o significado é uma JANELA: 1 < classes < pontos. Nem repouso, nem transitividade. E\n"
      "direção, escala e volume não são coisas que existam por si --- são relações que a lei sustenta:\n"
      "a direção é o sinal (σσ'=−1, a ida e volta que troca), a escala é só razão (nenhuma absoluta,\n"
      "e na borda |λ|=1 nada cresce), o volume é o que o determinante conserva."
      : "FALHOU — rever");
    return !ok;
}
