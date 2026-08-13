/* conecthus/backends/glsl/filtrar.c — GLSL realiza «pintar» no predicado do filtro.
 * Assinatura (1,0,1). dep = (rel != 0) & (existe != 0).
 * Layout int[]: rel[0..n), existe[n..2n), dep[2n..3n). */
unsigned char arena[65536];

int filtrar(int n){
    int *a = (int *)arena;
    for(int i = 0; i < n; i++){
        int rel = a[i];
        int ex = a[n + i];
        a[2 * n + i] = (rel != 0 && ex != 0) ? 1 : 0;
    }
    return n;
}
