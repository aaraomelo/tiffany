/* pgm.h — o leitor PGM binário (P5) reusado: lê uma imagem para um buffer de bytes. */
#ifndef PGM_H
#define PGM_H
#include <stdio.h>
#include <stdlib.h>

static unsigned char *le_pgm(const char *path, int *w, int *h){
    FILE *f=fopen(path,"rb"); if(!f) return NULL;
    char mg[3]={0}; if(fscanf(f,"%2s",mg)!=1||mg[0]!='P'||mg[1]!='5'){ fclose(f); return NULL; }
    int mx, c=fgetc(f);
    while(c=='#'||c==' '||c=='\n'||c=='\t'||c=='\r'){ if(c=='#'){ while((c=fgetc(f))!='\n'&&c!=EOF); } c=fgetc(f); }
    ungetc(c,f);
    if(fscanf(f,"%d %d %d",w,h,&mx)!=3){ fclose(f); return NULL; } fgetc(f);
    unsigned char *px=malloc((size_t)(*w)*(*h));
    if(fread(px,1,(size_t)(*w)*(*h),f)!=(size_t)(*w)*(*h)){ free(px); fclose(f); return NULL; }
    fclose(f); return px;
}

#endif
