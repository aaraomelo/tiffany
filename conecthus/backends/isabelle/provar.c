/* conecthus/backends/isabelle/provar.c — Isabelle realiza «provar».
 * Assinatura (0,1,1). Fecha a volta: x⊕x=0 (XOR), residual 0. */
unsigned char arena[65536];

int provar_xor_nulo(int n){
    int *a = (int *)arena;
    int falhas = 0;
    for(int i = 0; i < n; i++){
        int x = a[i];
        if((x ^ x) != 0) falhas++;
        a[n + i] = x ^ x;   /* residual */
    }
    return falhas;
}
