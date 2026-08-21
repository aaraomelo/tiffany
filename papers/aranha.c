/*
 * neurônio — gato A=[[1,1],[1,0]]
 * Lê arquivo byte a byte (streaming). Espectro = [e+o, e]
 *   e = popcnt(byte & 0xAA)   bits em posições pares
 *   o = popcnt(byte & 0x55)   bits em posições ímpares
 *
 * Uso: ./neuronio CAMINHO
 * Saída: [total, e]\n
 */
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;

    FILE *f = fopen(argv[1], "rb");
    if (!f)
        return 1;

    long e = 0, o = 0;
    int c;

    while ((c = fgetc(f)) != EOF) {
        unsigned char b = (unsigned char)c;
        e += __builtin_popcount(b & 0xAAu);
        o += __builtin_popcount(b & 0x55u);
    }

    fclose(f);
    printf("[%ld,%ld]\n", e + o, e);
    return 0;
}
