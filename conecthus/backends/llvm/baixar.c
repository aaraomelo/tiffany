/* conecthus/backends/llvm/baixar.c — LLVM realiza «baixar».
 * Assinatura (1,0,1). Baixar = copiar bloco (IR → metal conceptual). */
unsigned char arena[65536];

int baixar(int in_off, int n, int out_off){
    for(int i = 0; i < n; i++)
        arena[out_off + i] = arena[in_off + i];
    return n;
}
