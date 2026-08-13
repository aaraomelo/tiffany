/* conecthus/core/execute.h — STEP→BACK→MEASURE→Result (Maestro/Metrónomo). */
#ifndef CONECTHUS_EXECUTE_H
#define CONECTHUS_EXECUTE_H
#include "../lang/claim.h"

/* Dados de entrada por família (não residem no Claim). */
typedef struct {
    int n;
    int v[32];   /* percentagens / depths / etc. */
} ClaimInput;

int claim_execute(const Claim *c, const ClaimInput *in, ClaimResult *out);

/* classificação textual → código */
int claim_class_of(const char *classify);

#endif
