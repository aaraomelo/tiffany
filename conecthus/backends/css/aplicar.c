/* conecthus/backends/css/aplicar.c — CSS realiza «aplicar» (pintar no DOM).
 * Assinatura (1,0,1). Entrada: linhas "prop:valor;" — saída normalizada para <style>. */
unsigned char arena[65536];

static int eh_esp(unsigned char c){
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

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

int css_aplicar(int in_off, int n, int out_off){
    int m = 0;
    int i = 0;
    while(i < n && m < 60000){
        while(i < n && eh_esp(arena[in_off + i])) i = i + 1;
        if(i >= n) break;
        int j = i;
        while(j < n && arena[in_off + j] != ';' && arena[in_off + j] != '\n') j = j + 1;
        if(j > i){
            if(m > 0){ arena[out_off + m] = ' '; m = m + 1; }
            int k = i;
            while(k < j && m < 60000){ arena[out_off + m] = arena[in_off + k]; m = m + 1; k = k + 1; }
            if(m < 60000){ arena[out_off + m] = ';'; m = m + 1; }
        }
        if(j < n && arena[in_off + j] == ';') j = j + 1;
        i = j;
    }
    return m;
}

int css_move(int in_off, int n, int out_off, int sentido){
    if(sentido < 0) return css_aplicar(in_off, n, out_off);
    return copia(in_off, n, out_off);
}
