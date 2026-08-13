/* conecthus/backends/dafny/decidir.c — Dafny realiza «decidir» (Φ pontes).
 * Assinatura (1,1,0) = trial. Layout arena como int[]:
 *   prof[0..n), epist[n..2n), cmd[2n..3n), est[3n..4n) */
unsigned char arena[65536];

int decidir(int n){
    int *a = (int *)arena;
    for(int i = 0; i < n; i++){
        int prof = a[i];
        int epist = a[n + i];
        a[2 * n + i] = prof > 3 ? 3 : prof;   /* comando */
        a[3 * n + i] = epist;                 /* estatuto */
    }
    return n;
}
