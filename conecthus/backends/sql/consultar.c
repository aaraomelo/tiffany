/* conecthus/backends/sql/consultar.c — SQL realiza «consultar» (porta da interface).
 * Assinatura (1,1,1): analisa, compila para forma interna, e o texto atravessa.
 * MOVE(sql,-1) compila; +1 descompõe — Lei 1. O pleno mora em banco/sql.c. */
unsigned char arena[65536];

static int eq_word(const unsigned char *s, int n, const char *w){
    int i = 0;
    while(w[i]){
        if(i >= n || s[i] != (unsigned char)w[i]) return 0;
        i = i + 1;
    }
    return 1;
}
static int eh_kw(const unsigned char *s, int n){
    return eq_word(s, n, "INSERT") || eq_word(s, n, "SELECT") || eq_word(s, n, "UPDATE") ||
           eq_word(s, n, "DELETE") || eq_word(s, n, "CREATE") || eq_word(s, n, "BUSCA") ||
           eq_word(s, n, "ACHA") || eq_word(s, n, "IMPORT") || eq_word(s, n, "TEXTO") ||
           eq_word(s, n, "FROM") || eq_word(s, n, "WHERE") || eq_word(s, n, "TABLE") ||
           eq_word(s, n, "INTO") || eq_word(s, n, "VALUES") || eq_word(s, n, "CORPO") ||
           eq_word(s, n, "LINGUAGENS");
}
static void emite_tag(int out_off, int *m, const unsigned char *s, int n){
    arena[out_off + *m] = '<'; *m = *m + 1;
    int k = 0;
    while(k < n){ arena[out_off + *m] = s[k]; *m = *m + 1; k = k + 1; }
    arena[out_off + *m] = '>'; *m = *m + 1;
}

int sql_compilar(int in_off, int n, int out_off){
    int m = 0, i = 0;
    while(i < n){
        while(i < n && (arena[in_off + i] == ' ' || arena[in_off + i] == '\t' ||
                        arena[in_off + i] == '\n' || arena[in_off + i] == '\r')) i = i + 1;
        if(i >= n) break;
        if(arena[in_off + i] == '\'' || arena[in_off + i] == '"'){
            unsigned char q = arena[in_off + i]; i = i + 1;
            arena[out_off + m] = '<'; m = m + 1;
            arena[out_off + m] = 'S'; m = m + 1;
            arena[out_off + m] = '>'; m = m + 1;
            while(i < n && arena[in_off + i] != q){
                arena[out_off + m] = arena[in_off + i]; m = m + 1; i = i + 1;
            }
            if(i < n) i = i + 1;
            arena[out_off + m] = '<'; m = m + 1;
            arena[out_off + m] = '/'; m = m + 1;
            arena[out_off + m] = 'S'; m = m + 1;
            arena[out_off + m] = '>'; m = m + 1;
        } else {
            int j = i;
            while(j < n && arena[in_off + j] > ' ') j = j + 1;
            int wlen = j - i;
            if(wlen <= 0) break;
            if(eh_kw(arena + in_off + i, wlen)) emite_tag(out_off, &m, arena + in_off + i, wlen);
            else {
                int k = 0;
                while(k < wlen){ arena[out_off + m] = arena[in_off + i + k]; m = m + 1; k = k + 1; }
            }
            i = j;
        }
    }
    return m;
}

int sql_descompilar(int in_off, int n, int out_off){
    int m = 0, i = 0;
    while(i < n){
        if(arena[in_off + i] == '<'){
            int j = i + 1;
            while(j < n && arena[in_off + j] != '>') j = j + 1;
            if(j >= n) break;
            int tlen = j - (i + 1);
            if(tlen == 1 && arena[in_off + i + 1] == 'S'){
                if(m > 0 && arena[out_off + m - 1] != ' ') { arena[out_off + m] = ' '; m = m + 1; }
                i = j + 1;
                arena[out_off + m] = '\''; m = m + 1;
                while(i < n){
                    if(arena[in_off + i] == '<' && i + 3 < n &&
                       arena[in_off + i + 1] == '/' && arena[in_off + i + 2] == 'S' &&
                       arena[in_off + i + 3] == '>'){
                        i = i + 4; break;
                    }
                    arena[out_off + m] = arena[in_off + i]; m = m + 1; i = i + 1;
                }
                arena[out_off + m] = '\''; m = m + 1;
            } else {
                if(tlen > 0 && m > 0){ arena[out_off + m] = ' '; m = m + 1; }
                int k = 0;
                while(k < tlen){ arena[out_off + m] = arena[in_off + i + 1 + k]; m = m + 1; k = k + 1; }
                i = j + 1;
            }
        } else {
            arena[out_off + m] = arena[in_off + i]; m = m + 1; i = i + 1;
        }
    }
    return m;
}

int sql_move(int in_off, int n, int out_off, int sentido){
    if(sentido < 0) return sql_compilar(in_off, n, out_off);
    return sql_descompilar(in_off, n, out_off);
}
