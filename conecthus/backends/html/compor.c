/* conecthus/backends/html/compor.c — HTML realiza «compor» (porta DOM).
 * Assinatura (1,1,1). MOVE(html,-1) compõe; +1 descompõe — Lei 1.
 * O HTML já é a forma interna; compor† = copiar (involução exacta). */
unsigned char arena[65536];

static int copia(int in_off, int n, int out_off){
    int m = 0;
    int i = 0;
    while(i < n && m < 60000){
        arena[out_off + m] = arena[in_off + i];
        m = m + 1;
        i = i + 1;
    }
    return m;
}

int html_compor(int in_off, int n, int out_off){
    return copia(in_off, n, out_off);
}

int html_descompor(int in_off, int n, int out_off){
    return copia(in_off, n, out_off);
}

int html_move(int in_off, int n, int out_off, int sentido){
    if(sentido < 0) return html_compor(in_off, n, out_off);
    return html_descompor(in_off, n, out_off);
}
