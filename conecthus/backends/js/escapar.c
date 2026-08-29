/* conecthus/backends/js/escapar.c — JS realiza «escapar» (script seguro no DOM).
 * Assinatura (1,0,1). Sobe: traduz → js_escapar.wasm */
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

int js_escapar(int in_off, int n, int out_off){
    int m = 0;
    int i = 0;
    while(i < n && m < 60000){
        unsigned char c = arena[in_off + i];
        if(c == '\\' || c == '"' || c == '\'' || c == '\n' || c == '\r' || c == '<' || c == '>'){
            arena[out_off + m] = '\\';
            m = m + 1;
            if(c == '\n'){ arena[out_off + m] = 'n'; m = m + 1; }
            else if(c == '\r'){ arena[out_off + m] = 'r'; m = m + 1; }
            else { arena[out_off + m] = c; m = m + 1; }
        } else {
            arena[out_off + m] = c;
            m = m + 1;
        }
        i = i + 1;
    }
    return m;
}

int js_move(int in_off, int n, int out_off, int sentido){
    if(sentido < 0) return js_escapar(in_off, n, out_off);
    return copia(in_off, n, out_off);
}
