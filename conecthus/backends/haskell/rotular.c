/* conecthus/backends/haskell/rotular.c — Haskell realiza «rotular».
 * Assinatura (0,0,1). Prefixo "grafo:" + alfanum do id, max 40 chars no rótulo. */
unsigned char arena[65536];

int rotular(int in_off, int n, int out_off){
    const char *pfx = "grafo:";
    int m = 0;
    for(int k = 0; pfx[k]; k++) arena[out_off + m++] = (unsigned char)pfx[k];
    int lim = 40;
    int feitos = 0;
    for(int i = 0; i < n && feitos < lim; i++){
        unsigned char c = arena[in_off + i];
        int ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                 (c >= '0' && c <= '9');
        if(ok){ arena[out_off + m++] = c; feitos++; }
    }
    return m;
}
