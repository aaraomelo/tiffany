/* conecthus/backends/ptx/emitir.c — PTX realiza «emitir».
 * Assinatura (1,0,1). Escreve bytes no metal (arena); volta lê. */
unsigned char arena[65536];

int emitir(int in_off, int n, int out_off){
    for(int i = 0; i < n; i++)
        arena[out_off + i] = arena[in_off + i];
    return n;
}

int emitir_volta(int in_off, int n, int out_off){
    return emitir(in_off, n, out_off);
}
