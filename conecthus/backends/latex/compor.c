/* conecthus/backends/latex/compor.c — LaTeX realiza «compor» (núcleo; tex.wasm é o pleno).
 * Assinatura (1,1,1). MOVE(latex,-1) compõe; +1 descompõe — Lei 1. */
unsigned char arena[65536];

int latex_compor(int in_off, int n, int out_off){
    int m = 0;
    for(int i = 0; i < n && m < 60000; ){
        unsigned char c = arena[in_off + i];
        if(c == '\\' && i + 1 < n){
            int j = i + 1;
            while(j < n){
                unsigned char d = arena[in_off + j];
                if(!((d >= 'a' && d <= 'z') || (d >= 'A' && d <= 'Z'))) break;
                j++;
            }
            arena[out_off + m++] = '<';
            for(int k = i + 1; k < j; k++) arena[out_off + m++] = arena[in_off + k];
            arena[out_off + m++] = '>';
            i = j;
            continue;
        }
        arena[out_off + m++] = c;
        i++;
    }
    return m;
}

int latex_descompor(int in_off, int n, int out_off){
    int m = 0;
    for(int i = 0; i < n && m < 60000; ){
        unsigned char c = arena[in_off + i];
        if(c == '<'){
            int j = i + 1;
            while(j < n && arena[in_off + j] != '>') j++;
            arena[out_off + m++] = '\\';
            for(int k = i + 1; k < j; k++) arena[out_off + m++] = arena[in_off + k];
            i = j < n ? j + 1 : j;
            continue;
        }
        arena[out_off + m++] = c;
        i++;
    }
    return m;
}
