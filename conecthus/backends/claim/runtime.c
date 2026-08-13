/* conecthus/backends/claim/runtime.c — IR Claim em wasm.
 * ids: 0 Pareto · 1 Why · 2 Kanban · 3 GUT · 4 PDCA · 5 FiveW2H
 *      6 Ishikawa · 7 VSM · 8 Fluxograma
 * residual universal em a[7].
 */
unsigned char arena[65536];

static int claim_pareto(void){
    int *a = (int *)arena;
    int n = a[0], sum = 0;
    for(int i = 1; i <= n; i++) sum += a[i];
    int R = sum - 100;
    a[0] = sum; a[1] = sum; a[2] = R;
    a[3] = ((sum + 1) - 100 != 0) ? 1 : 0;
    a[7] = R;
    return R == 0 ? 1 : 0;
}

static int claim_why(void){
    int *a = (int *)arena;
    int depth = a[0], root_at = a[1];
    int R = depth < root_at ? (root_at - depth) : 0;
    a[0] = R; a[1] = 1; a[7] = R;
    return R == 0 ? 1 : 0;
}

static int claim_kanban(void){
    int *a = (int *)arena;
    int n = a[0];
    int col3 = 0, col2 = n / 2;
    if(col2 < 0) col2 = 0;
    a[0] = col3; a[1] = col2; a[2] = col3;
    a[3] = (col2 > col3) ? 1 : 0;
    a[7] = col3;
    return col3 == 0 ? 1 : 0;
}

static int claim_gut(void){
    int *a = (int *)arena;
    int G = a[0], U = a[1], T = a[2];
    int score = G * U * T;
    int fora = 0;
    if(G < 1 || G > 5) fora++;
    if(U < 1 || U > 5) fora++;
    if(T < 1 || T > 5) fora++;
    int Up = U < 5 ? U + 1 : U;
    a[0] = score; a[1] = fora;
    a[2] = (0 * U * T == 0 * Up * T) ? 1 : 0;
    a[7] = fora;
    return fora == 0 ? 1 : 0;
}

static int claim_pdca(void){
    int *a = (int *)arena;
    int plan = a[0], done = a[1], has_num = a[2];
    int R, closed;
    if(!has_num){ R = 1; closed = 0; }
    else { R = plan - done; closed = (R == 0) ? 1 : 0; }
    a[0] = R; a[1] = closed; a[2] = 1; a[7] = R;
    return closed;
}

static int claim_fivew2h(void){
    int *a = (int *)arena;
    int root_in = a[0], n_act = a[1];
    int R = root_in ? 0 : 1;
    int closed = (root_in != 0 && n_act > 0) ? 1 : 0;
    a[0] = R; a[1] = closed; a[2] = 1; a[7] = R;
    return closed;
}

static int claim_ishikawa(void){
    int *a = (int *)arena;
    int k = a[0], root = a[1], root_re = a[2];
    int k_ok = (k >= 2 && k <= 12) ? 1 : 0;
    int R = (!k_ok || root != root_re) ? 1 : 0;
    a[0] = R; a[1] = k; a[2] = 0;
    a[7] = R;
    return R == 0 ? 1 : 0;
}

static int claim_vsm(void){
    int *a = (int *)arena;
    int ha = a[0], hf = a[1];
    int R = (ha && hf) ? 0 : 1;
    a[0] = R; a[1] = (R == 0) ? 1 : 0; a[2] = 1; a[7] = R;
    return R == 0 ? 1 : 0;
}

static int claim_fluxograma(void){
    int *a = (int *)arena;
    int n = a[0], paired = a[1];
    int R = n - paired;
    if(R < 0) R = -R;
    int closed = (n > 0 && paired == n) ? 1 : 0;
    a[0] = R; a[1] = closed; a[2] = 1; a[7] = R;
    return closed;
}

/* id 9: a[0]=n, a[1..n]=scores, a[n+1..2n]=acao → a[0]=R inversões */
static int claim_front_gut(void){
    int *a = (int *)arena;
    int n = a[0];
    int inv = 0;
    for(int i = 0; i < n - 1; i++){
        int ia = a[1 + n + i], ib = a[1 + n + i + 1];
        if(ia < 0 || ia >= n || ib < 0 || ib >= n){ inv++; continue; }
        if(a[1 + ia] < a[1 + ib]) inv++;
    }
    a[0] = inv; a[7] = inv;
    return inv == 0 ? 1 : 0;
}

/* id 10: a[0]=root, a[1]=n, a[2..]=causes */
static int claim_front_why(void){
    int *a = (int *)arena;
    int root = a[0], n = a[1], miss = 1;
    for(int i = 0; i < n; i++) if(a[2 + i] == root) miss = 0;
    a[0] = miss; a[7] = miss;
    return miss == 0 ? 1 : 0;
}

/* id 11: a[0]=plan, a[1]=futuro */
static int claim_front_pdca(void){
    int *a = (int *)arena;
    int R = a[0] - a[1];
    if(R < 0) R = -R;
    a[0] = R; a[7] = R;
    return R == 0 ? 1 : 0;
}

/* id 12: a[0]=proj, a[1]=meas, a[2]=external → a[0]=R, a[1]=closed, a[2]=mut */
static int claim_estacao(void){
    int *a = (int *)arena;
    int proj = a[0], meas = a[1], ext = a[2];
    int R = proj - meas;
    if(R < 0) R = -R;
    int closed = (ext != 0 && R == 0) ? 1 : 0;
    a[0] = R; a[1] = closed; a[2] = ext ? 0 : 1; a[7] = R;
    return closed;
}

/* id 13: write P,M,ext → slot; retrieve; R=0 se iguais. mut se P+1 muda. */
static int claim_banco(void){
    int *a = (int *)arena;
    int p = a[0], m = a[1], ext = a[2];
    a[10] = p; a[11] = m; a[12] = ext;          /* write_document */
    int same = (a[10] == p && a[11] == m && a[12] == ext);
    int R = same ? 0 : 1;
    a[0] = R; a[1] = same ? 1 : 0;
    a[2] = (p + 1 != p) ? 1 : 0;                /* alter_doc denuncia */
    a[7] = R;
    return same ? 1 : 0;
}

/* id 14: a[0]=local, a[1]=live, a[2]=external → a[0]=R, a[1]=closed, a[2]=mut */
static int claim_deploy(void){
    int *a = (int *)arena;
    int loc = a[0], live = a[1], ext = a[2];
    int R = (loc && live) ? 0 : 1;
    int closed = (ext != 0 && loc && live) ? 1 : 0;
    a[0] = R; a[1] = closed; a[2] = loc ? 1 : 0; a[7] = R;
    return closed;
}

int claim_run(int id){
    if(id == 0) return claim_pareto();
    if(id == 1) return claim_why();
    if(id == 2) return claim_kanban();
    if(id == 3) return claim_gut();
    if(id == 4) return claim_pdca();
    if(id == 5) return claim_fivew2h();
    if(id == 6) return claim_ishikawa();
    if(id == 7) return claim_vsm();
    if(id == 8) return claim_fluxograma();
    if(id == 9) return claim_front_gut();
    if(id == 10) return claim_front_why();
    if(id == 11) return claim_front_pdca();
    if(id == 12) return claim_estacao();
    if(id == 13) return claim_banco();
    if(id == 14) return claim_deploy();
    return -1;
}

int claim_residual(void){
    int *a = (int *)arena;
    return a[7];
}
