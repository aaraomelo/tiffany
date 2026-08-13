/* conecthus/lang/claim.h — AST da IR Claim (fonte). Result é observação, não campo do Claim. */
#ifndef CONECTHUS_CLAIM_H
#define CONECTHUS_CLAIM_H

#define CLAIM_MAX 16

typedef struct {
    char name[64];
    int  law;
    char object[64];
    char step[128];
    char back[128];
    char measure[128];
    char invariant[128];
    char mutate[128];
    char classify[64];
} Claim;

/* Result — só depois de STEP→BACK→MEASURE (nunca escrito no .claim) */
typedef struct {
    long forward;
    long reverse;
    long residual;
    int  mutation;   /* 0=SURVIVED 1=BROKEN 2=UNTESTED */
    int  klass;      /* 0=STRUCTURAL 1=CONVENTIONAL 2=REALIZATION 3=HYPOTHESIS 4=FALSE */
    int  closed;     /* 1 se R==0 */
} ClaimResult;

/* parse: devolve 0 se OK; -1 erro. Não preenche residual. */
int claim_parse(const char *src, Claim *out);
int claim_parse_file(const char *path, Claim *out);

/* serializa AST → texto canónico (volta do parser) */
int claim_emit(const Claim *c, char *out, int cap);

#endif
