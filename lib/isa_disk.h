/* isa_disk.h — A ISA no disco: MOVE(destino, sentido), uma função, um laço.
 *
 * Mesma semântica que tools/isa.c (traduz → assets/figuras/wasm/isa.wasm).
 * O estado vive nos slots {total, e}; ESQUILO = ×i ordem 4, TROCA = J ordem 2.
 */
#ifndef ISA_DISK_H
#define ISA_DISK_H

static long isa_M[8192];

#define ISA_S_A       1024
#define ISA_S_B       1025
#define ISA_S_R       1026
#define ISA_S_SOMA    1040
#define ISA_S_SUB     1041
#define ISA_S_E       1042
#define ISA_S_OU      1043
#define ISA_S_XOU     1044
#define ISA_S_GOLD    1045
#define ISA_S_NEGRO   1046
#define ISA_S_ESQUILO 1047
#define ISA_S_TROCA   1048
#define ISA_S_CMP     1049

static inline void isa_word(long slot, long t, long e){
    isa_M[2*slot] = t;
    isa_M[2*slot + 1] = e;
}

static inline void isa_read(long slot, long *t, long *e){
    *t = isa_M[2*slot];
    *e = isa_M[2*slot + 1];
}

static long isa_MOVE(long destino, long sentido){
    if(sentido < 0){
        isa_M[2*destino] = isa_M[2*ISA_S_R];
        isa_M[2*destino + 1] = isa_M[2*ISA_S_R + 1];
        return isa_M[2*ISA_S_R];
    }

    long t = 0, e = 0;

    if(destino < ISA_S_SOMA){
        isa_M[2*ISA_S_B] = isa_M[2*ISA_S_A];
        isa_M[2*ISA_S_B + 1] = isa_M[2*ISA_S_A + 1];
        isa_M[2*ISA_S_A] = isa_M[2*destino];
        isa_M[2*ISA_S_A + 1] = isa_M[2*destino + 1];
        return isa_M[2*ISA_S_A];
    }
    {
        long at = isa_M[2*ISA_S_A], ae = isa_M[2*ISA_S_A + 1];
        long bt = isa_M[2*ISA_S_B], be = isa_M[2*ISA_S_B + 1];
        long x[2], y[2], cin = 0;
        int conta = 1;

        if(destino == ISA_S_SOMA){ x[0]=at; y[0]=bt; x[1]=ae; y[1]=be; }
        else if(destino == ISA_S_SUB){ x[0]=at; y[0]=~bt; x[1]=ae; y[1]=~be; cin=1; }
        else if(destino == ISA_S_GOLD){ x[0]=at; y[0]=ae; x[1]=0; y[1]=at; }
        else if(destino == ISA_S_NEGRO){ x[0]=0; y[0]=ae; x[1]=at; y[1]=~ae; cin=1; }
        else if(destino == ISA_S_ESQUILO){ x[0]=0; y[0]=~ae; x[1]=0; y[1]=at; cin=1; }
        else conta = 0;

        if(conta){
            for(int k = 0; k < 2; k++){
                long r = 0, c = cin;
                for(int i = 0; i < 64; i++){
                    long a1 = (x[k] >> i) & 1, b1 = (y[k] >> i) & 1;
                    long sm = a1 ^ b1 ^ c;
                    c = (a1 & b1) | (c & (a1 ^ b1));
                    r |= sm << i;
                }
                if(k == 0) t = r; else e = r;
            }
            if(destino == ISA_S_ESQUILO) e = at;
            if(destino == ISA_S_GOLD) e = at;
            if(destino == ISA_S_NEGRO) t = ae;
        }
        else if(destino == ISA_S_E){
            t = ~(~(at & bt) & ~(at & bt));
            e = ~(~(ae & be) & ~(ae & be));
        }
        else if(destino == ISA_S_OU){
            t = ~(~(at & at) & ~(bt & bt));
            e = ~(~(ae & ae) & ~(be & be));
        }
        else if(destino == ISA_S_XOU){
            t = ~(~(at & ~(at & bt)) & ~(bt & ~(at & bt)));
            e = ~(~(ae & ~(ae & be)) & ~(be & ~(ae & be)));
        }
        else if(destino == ISA_S_TROCA){ t = ae; e = at; }
        else if(destino == ISA_S_CMP){
            long f = 0;
            if(!at && !ae && !bt && !be) f++;
            if(at == bt && ae == be) f += 2;
            else if(at < bt) f += 4;
            isa_M[2*1028] = f;
            return f;
        }
    }
    if(destino == ISA_S_GOLD || destino == ISA_S_NEGRO
       || destino == ISA_S_ESQUILO || destino == ISA_S_TROCA){
        isa_M[2*ISA_S_A] = t;
        isa_M[2*ISA_S_A + 1] = e;
    }
    isa_M[2*ISA_S_R] = t;
    isa_M[2*ISA_S_R + 1] = e;
    return t;
}

/* período de destino giratório: ESQUILO ou TROCA */
static inline int isa_periodo_giro(long dest){
    long t0, e0;
    isa_word(ISA_S_A, 1, 0);
    isa_read(ISA_S_A, &t0, &e0);
    for(int k = 1; k <= 8; k++){
        isa_MOVE(dest, 1);
        long t, e;
        isa_read(ISA_S_A, &t, &e);
        if(t == t0 && e == e0) return k;
    }
    return 0;
}

/* norma N = t² + e² no círculo unitário em ℤ (para ae=0) */
static inline long isa_norma2(long t, long e){ return t*t + e*e; }

#endif
