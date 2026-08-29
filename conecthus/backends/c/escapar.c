/* conecthus/backends/c/escapar.c — C realiza «escapar» (Φ pontes).
 * Roupa: não copia a ULA/gato de banco/sql.c (o motor).
 * Assinatura no banco: (1,0,1). Sobe: traduz → escapar.wasm
 * Contrato host: offsets relativos a arena[]; memória exportada DISCO (base NULO=8). */
unsigned char arena[65536];

int escapar(int in_off, int n, int out_off){
    int m = 0;
    for(int i = 0; i < n; i++){
        unsigned char c = arena[in_off + i];
        if(c == '\\' || c == '{' || c == '}' || c == '$' || c == '&' ||
           c == '#' || c == '%' || c == '_' || c == '^' || c == '~')
            arena[out_off + m++] = '\\';
        arena[out_off + m++] = c;
    }
    return m;
}
