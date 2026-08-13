/* conecthus/backends/control/runtime.c — Controlo em wasm (acima da IR).
 * Assinatura (1,1,0): selecciona. Arena int[].
 *
 * Layout (8 eixos):
 *   a[0]=exige_fecho
 *   a[1..8]=theta[0..7]
 *   a[9]=R_cand
 *   a[10..17]=D[0..7]
 * Saída: a[18]=acção (0 RETAIN, 1 MOVE, 2 RETRACT); return=acção
 *
 * Eixos: L1, |R|, |n|, mut, caixa, teclado, fonético, forma
 * (dist_arena preenche L1..forma numérica; teclado/φ ficam 0 sem texto —
 *  texto real mede-se em eixos_texto.c / controlo.js.)
 */
unsigned char arena[65536];

int control_decide(void){
    int *a = (int *)arena;
    int exige = a[0];
    int R = a[9];
    if(exige && R != 0){
        a[18] = 2;   /* RETRACT */
        return 2;
    }
    int ok = 1;
    for(int i = 0; i < 8; i++){
        if(a[10 + i] > a[1 + i]) ok = 0;
    }
    a[18] = ok ? 0 : 1;
    return a[18];
}

/* a[20]=n_a, a[21..]=va; a[40]=n_b, a[41..]=vb; a[60]=Ra,a[61]=Rb; a[62]=muta,a[63]=mutb
 * escreve a[10..17]; teclado/fonético = 0 (sem buffer de texto no subset C) */
int control_dist_arena(void){
    int *a = (int *)arena;
    int na = a[20], nb = a[40];
    int n = na < nb ? na : nb;
    int l1 = 0, forma = 0;
    for(int i = 0; i < n; i++){
        int d = a[21 + i] - a[41 + i];
        if(d < 0) d = -d;
        l1 += d;
        if(d > forma) forma = d;
    }
    for(int i = n; i < na; i++){ int d = a[21 + i]; if(d < 0) d = -d; l1 += d; }
    for(int i = n; i < nb; i++){ int d = a[41 + i]; if(d < 0) d = -d; l1 += d; }
    int caixa = a[21] - a[41]; if(caixa < 0) caixa = -caixa;
    int dR = a[60] - a[61]; if(dR < 0) dR = -dR;
    int dn = na - nb; if(dn < 0) dn = -dn;
    int dm = a[62] - a[63]; if(dm < 0) dm = -dm;
    a[10] = l1;
    a[11] = dR;
    a[12] = dn;
    a[13] = dm;
    a[14] = caixa;
    a[15] = 0;      /* teclado — texto em eixos_texto / JS */
    a[16] = 0;      /* fonético */
    a[17] = forma;
    return l1;
}
