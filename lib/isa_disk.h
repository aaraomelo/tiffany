/* isa_disk.h — MOVE; estado = Word_8 por slot (1 byte).
 *
 * Par (t,e) = dois slots consecutivos. ALU em 8 bits.
 * Sem long C: índices e valores = unsigned / Word_8 / int8 no envelope. */
#ifndef ISA_DISK_H
#define ISA_DISK_H

#include <stdint.h>

static uint8_t isa_M[16384];
static unsigned isa_w8_wrap = 0;

/* Registos: cada um = dois slots (total, e). */
#define ISA_S_A       1024
#define ISA_S_B       1026
#define ISA_S_R       1028
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

static inline uint8_t isa_w8(int v){
    if(v < -128 || v > 255) isa_w8_wrap++;
    return (uint8_t)(int8_t)v;
}

static inline void isa_put(unsigned slot, uint8_t v){
    isa_M[slot] = v;
}

static inline uint8_t isa_get(unsigned slot){
    return isa_M[slot];
}

static inline void isa_word(unsigned slot, int t, int e){
    isa_put(slot,     isa_w8(t));
    isa_put(slot + 1, isa_w8(e));
}

/* Lê o envelope como int8 (visão assinada local — não é long semântico). */
static inline void isa_read(unsigned slot, int *t, int *e){
    *t = (int)(int8_t)isa_get(slot);
    *e = (int)(int8_t)isa_get(slot + 1);
}

static uint8_t isa_MOVE(unsigned destino, int sentido){
    if(sentido < 0){
        isa_M[destino]     = isa_M[ISA_S_R];
        isa_M[destino + 1] = isa_M[ISA_S_R + 1];
        return isa_M[ISA_S_R];
    }

    uint8_t t = 0, e = 0;

    if(destino < ISA_S_SOMA){
        isa_M[ISA_S_B]     = isa_M[ISA_S_A];
        isa_M[ISA_S_B + 1] = isa_M[ISA_S_A + 1];
        isa_M[ISA_S_A]     = isa_M[destino];
        isa_M[ISA_S_A + 1] = isa_M[destino + 1];
        return isa_M[ISA_S_A];
    }
    {
        uint8_t at = isa_M[ISA_S_A], ae = isa_M[ISA_S_A + 1];
        uint8_t bt = isa_M[ISA_S_B], be = isa_M[ISA_S_B + 1];
        uint8_t x[2], y[2], cin = 0;
        int conta = 1;

        if(destino == ISA_S_SOMA){ x[0]=at; y[0]=bt; x[1]=ae; y[1]=be; }
        else if(destino == ISA_S_SUB){ x[0]=at; y[0]=(uint8_t)~bt; x[1]=ae; y[1]=(uint8_t)~be; cin=1; }
        else if(destino == ISA_S_GOLD){ x[0]=at; y[0]=ae; x[1]=0; y[1]=at; }           /* ×σ, σ²=σ+1 */
        else if(destino == ISA_S_NEGRO){ x[0]=0; y[0]=ae; x[1]=at; y[1]=(uint8_t)~ae; cin=1; }
        else if(destino == ISA_S_ESQUILO){ x[0]=0; y[0]=(uint8_t)~ae; x[1]=0; y[1]=at; cin=1; }
        else conta = 0;

        if(conta){
            for(int k = 0; k < 2; k++){
                uint8_t r = 0, c = cin;
                for(int i = 0; i < 8; i++){
                    uint8_t a1 = (x[k] >> i) & 1u, b1 = (y[k] >> i) & 1u;
                    uint8_t sm = (uint8_t)(a1 ^ b1 ^ c);
                    c = (uint8_t)((a1 & b1) | (c & (a1 ^ b1)));
                    r = (uint8_t)(r | (sm << i));
                }
                if(k == 0) t = r; else e = r;
            }
            if(destino == ISA_S_ESQUILO) e = at;
            if(destino == ISA_S_GOLD) e = at;
            if(destino == ISA_S_NEGRO) t = ae;
        }
        else if(destino == ISA_S_E){
            t = (uint8_t)(at & bt);
            e = (uint8_t)(ae & be);
        }
        else if(destino == ISA_S_OU){
            t = (uint8_t)(at | bt);
            e = (uint8_t)(ae | be);
        }
        else if(destino == ISA_S_XOU){
            t = (uint8_t)(at ^ bt);
            e = (uint8_t)(ae ^ be);
        }
        else if(destino == ISA_S_TROCA){ t = ae; e = at; }
        else if(destino == ISA_S_CMP){
            uint8_t f = 0;
            if(!at && !ae && !bt && !be) f++;
            if(at == bt && ae == be) f = (uint8_t)(f + 2);
            else if(at < bt) f = (uint8_t)(f + 4);
            isa_M[1030] = f;
            return f;
        }
    }
    if(destino == ISA_S_GOLD || destino == ISA_S_NEGRO
       || destino == ISA_S_ESQUILO || destino == ISA_S_TROCA){
        isa_M[ISA_S_A]     = t;
        isa_M[ISA_S_A + 1] = e;
    }
    isa_M[ISA_S_R]     = t;
    isa_M[ISA_S_R + 1] = e;
    return t;
}

/* norma N = t² + e² do par lido do envelope. O quadrado SAI do átomo — t,e vêm de
 * `isa_read` como int8 e t²+e² pede 16 bits —, logo a norma é uma PROMOÇÃO e não um
 * valor de slot: devolve-se no andar de cima e não se escreve em Word_8 nenhuma. */
static inline int isa_norma2(int t, int e){ return t*t + e*e; }

static inline int isa_periodo_giro(unsigned dest){
    int t0, e0, t, e;
    isa_word(ISA_S_A, 1, 0);
    isa_read(ISA_S_A, &t0, &e0);
    for(int k = 1; k <= 8; k++){
        isa_MOVE(dest, 1);
        isa_read(ISA_S_A, &t, &e);
        if(t == t0 && e == e0) return k;
    }
    return 0;
}

#endif
