/* conecthus/backends/wasm/empilhar.c — Wasm realiza «empilhar».
 * Assinatura (1,0,1). Pilha de i32 em arena; push/pop involutivos. */
unsigned char arena[65536];

/* arena[0..3] = sp (int); valores a partir de arena[4] */
int empilhar_push(int v){
    int *a = (int *)arena;
    int sp = a[0];
    a[1 + sp] = v;
    a[0] = sp + 1;
    return sp + 1;
}

int empilhar_pop(void){
    int *a = (int *)arena;
    int sp = a[0];
    if(sp <= 0) return 0;
    a[0] = sp - 1;
    return a[sp];
}
