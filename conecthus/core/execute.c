/* conecthus/core/execute.c — executor: Claim + Input → Result (observa R).
 * Mutação fora do caminho normal: corre depois, preenche result.mutation. */
#include "execute.h"
#include "fronteira.h"
#include <string.h>

int claim_class_of(const char *classify){
    if(!classify) return 3;
    if(strstr(classify, "structural")) return 0;
    if(strstr(classify, "convention")) return 1;
    if(strstr(classify, "realization")) return 2;
    if(strstr(classify, "hypothesis")) return 3;
    if(strstr(classify, "false")) return 4;
    return 3;
}

static int exec_pareto(const ClaimInput *in, ClaimResult *r){
    int n = in->n;
    if(n < 1 || n > 32) return -1;
    long sum = 0;
    for(int i = 0; i < n; i++) sum += in->v[i];
    r->forward = sum;
    r->reverse = sum;                 /* recompute */
    r->residual = sum - 100;
    r->closed = (r->residual == 0);
    /* mutação: +1 no primeiro */
    long Rm = (sum + 1) - 100;
    r->mutation = (Rm != 0) ? 1 : 0;  /* BROKEN esperado se era fecho */
    r->klass = 0;
    return 0;
}

static int exec_why(const ClaimInput *in, ClaimResult *r){
    int depth = in->v[0];
    int root_at = in->v[1];
    r->forward = depth;
    r->reverse = root_at;
    r->residual = depth < root_at ? (root_at - depth) : 0;
    r->closed = (r->residual == 0);
    r->mutation = 2;                  /* profundidade = convention: UNTESTED estrutural */
    r->klass = 1;                     /* CONVENTIONAL depth */
    return 0;
}

static int exec_kanban(const ClaimInput *in, ClaimResult *r){
    int n = in->n > 0 ? in->n : in->v[0];
    long col3 = 0;
    long col2 = n / 2;
    r->forward = col3;
    r->reverse = col3;                /* invert_flow sem colisão */
    r->residual = col3;
    r->closed = (col3 == 0);
    r->mutation = (col2 > col3) ? 1 : 0;
    r->klass = 0;
    return 0;
}

static int exec_pipeline(const Claim *c, ClaimResult *r){
    /* PipelineClosure: emit ∘ parse ≈ id (fonte ↔ reconstruída) */
    char buf[1024];
    Claim c2;
    int n = claim_emit(c, buf, (int)sizeof buf);
    if(n < 0) return -1;
    if(claim_parse(buf, &c2) != 0) return -1;
    int same = strcmp(c->name, c2.name) == 0 && c->law == c2.law
            && strcmp(c->step, c2.step) == 0 && strcmp(c->back, c2.back) == 0
            && strcmp(c->measure, c2.measure) == 0;
    r->forward = same ? 0 : 1;
    r->reverse = 0;
    r->residual = same ? 0 : 1;
    r->closed = same;
    /* mutação: alterar AST e esperar residual ≠ 0 */
    Claim mut = *c;
    mut.law = c->law + 1;
    char bufm[1024]; Claim c3;
    claim_emit(&mut, bufm, (int)sizeof bufm);
    claim_parse(bufm, &c3);
    int broken = (c3.law != c->law);
    r->mutation = broken ? 1 : 0;
    r->klass = 0;
    return 0;
}

/* GUT = produto G×U×T: ordena (monótono em 1..5); 0 quebra monotonia (lean_cruz §L2). */
static int exec_gut(const ClaimInput *in, ClaimResult *r){
    if(in->n < 3) return -1;
    int G = in->v[0], U = in->v[1], T = in->v[2];
    long score = (long)G * (long)U * (long)T;
    r->forward = score;
    r->reverse = (long)G * (long)U * (long)T;   /* recompute */
    int fora = 0;
    if(G < 1 || G > 5) fora++;
    if(U < 1 || U > 5) fora++;
    if(T < 1 || T > 5) fora++;
    r->residual = fora + (r->forward != r->reverse ? 1 : 0);
    r->closed = (r->residual == 0);
    /* mutação allow_zero: produto com 0 é flat — subir U não sobe score */
    long s0 = 0L * (long)U * (long)T;
    long s1 = 0L * (long)(U < 5 ? U + 1 : U) * (long)T;
    r->mutation = (s0 == s1) ? 1 : 0;            /* BROKEN esperado */
    r->klass = 0;
    return 0;
}

/* PDCA: fecha pela leitura no Check — Plan precisa de alvo numérico (fundamento). */
static int exec_pdca(const ClaimInput *in, ClaimResult *r){
    if(in->n < 3) return -1;
    int plan = in->v[0], done = in->v[1], has_num = in->v[2];
    r->forward = plan;
    r->reverse = done;
    if(!has_num){
        r->residual = 1;                         /* residual ilegível */
        r->closed = 0;
        r->mutation = 1;                         /* drop_numeric → BROKEN */
    } else {
        r->residual = (long)plan - (long)done;
        r->closed = (r->residual == 0);
        r->mutation = 1;                         /* mutação drop_numeric denuncia */
    }
    r->klass = 0;
    return 0;
}

/* 5W2H = dilatação δ: da raiz às ações; par do Why (erosão). root ∈ actions. */
static int exec_fivew2h(const ClaimInput *in, ClaimResult *r){
    if(in->n < 2) return -1;
    int root_in = in->v[0];   /* 1 se a raiz do Why está no plano */
    int n_act  = in->v[1];
    r->forward = n_act;
    r->reverse = root_in;
    r->residual = root_in ? 0 : 1;
    r->closed = (root_in != 0) && (n_act > 0);
    /* mutação drop_root: tira a raiz → containment quebra */
    r->mutation = 1;          /* BROKEN esperado */
    r->klass = 0;
    return 0;
}

/* Ishikawa: k∈2..12 é LIVRE — escolha por causa estável sob re-partição (lean_convencao §V1). */
static int exec_ishikawa(const ClaimInput *in, ClaimResult *r){
    if(in->n < 3) return -1;
    int k = in->v[0];
    int root = in->v[1];
    int root_re = in->v[2];   /* após mudar k */
    r->forward = k;
    r->reverse = root;
    int k_ok = (k >= 2 && k <= 12);
    r->residual = (!k_ok || root != root_re) ? 1 : 0;
    r->closed = (r->residual == 0);
    r->mutation = 0;          /* SURVIVED — mutar k não quebra */
    r->klass = 1;             /* CONVENTIONAL */
    return 0;
}

/* VSM: dual actual↔futuro — os dois membros nomeados (fundamento). */
static int exec_vsm(const ClaimInput *in, ClaimResult *r){
    if(in->n < 2) return -1;
    int ha = in->v[0], hf = in->v[1];
    r->forward = ha;
    r->reverse = hf;
    r->residual = (ha && hf) ? 0 : 1;
    r->closed = (r->residual == 0);
    r->mutation = 1;          /* drop_futuro → BROKEN */
    r->klass = 0;
    return 0;
}

/* Fluxograma: dual nó a nó — cada nó actual tem futuro (e conversa). */
static int exec_fluxograma(const ClaimInput *in, ClaimResult *r){
    if(in->n < 2) return -1;
    int n = in->v[0], paired = in->v[1];
    r->forward = n;
    r->reverse = paired;
    long d = (long)n - (long)paired;
    if(d < 0) d = -d;
    r->residual = d;
    r->closed = (n > 0 && paired == n);
    r->mutation = 1;          /* drop_futuro_node → BROKEN */
    r->klass = 0;
    return 0;
}

/* Fronteira GUT↔5W2H: v[0]=n, v[1..n]=scores, v[n+1..2n]=acao (índices). */
static int exec_front_gut(const ClaimInput *in, ClaimResult *r){
    if(in->n < 1) return -1;
    int n = in->v[0];
    if(n < 2 || in->n < 1 + 2 * n) return -1;
    const int *score = &in->v[1];
    const int *acao = &in->v[1 + n];
    long inv = fronteira_gut_5w2h(score, acao, n);
    r->forward = n;
    r->reverse = inv;
    r->residual = inv;
    r->closed = (inv == 0);
    /* mutação: se há scores distintos adjacentes na ordem, swap quebraria */
    int can_break = 0;
    for(int i = 0; i < n - 1; i++){
        int a = acao[i], b = acao[i + 1];
        if(a >= 0 && a < n && b >= 0 && b < n && score[a] != score[b]) can_break = 1;
    }
    r->mutation = can_break ? 1 : 2;
    r->klass = 0;
    return 0;
}

/* Fronteira Why↔Ishikawa: v[0]=root, v[1]=n, v[2..]=causes. */
static int exec_front_why(const ClaimInput *in, ClaimResult *r){
    if(in->n < 2) return -1;
    int root = in->v[0], n = in->v[1];
    if(n < 1 || in->n < 2 + n) return -1;
    long miss = fronteira_why_ishikawa(root, &in->v[2], n);
    r->forward = root;
    r->reverse = n;
    r->residual = miss;
    r->closed = (miss == 0);
    r->mutation = 1; /* drop_root_from_map */
    r->klass = 0;
    return 0;
}

/* Fronteira PDCA↔VSM: v[0]=plan, v[1]=futuro. */
static int exec_front_pdca(const ClaimInput *in, ClaimResult *r){
    if(in->n < 2) return -1;
    long d = fronteira_pdca_vsm(in->v[0], in->v[1]);
    r->forward = in->v[0];
    r->reverse = in->v[1];
    r->residual = d;
    r->closed = (d == 0);
    r->mutation = 1; /* alter_futuro */
    r->klass = 0;
    return 0;
}

/* Estação: v[0]=projectado, v[1]=medido, v[2]=external (1=MES/estação, 0=réu). */
static int exec_estacao(const ClaimInput *in, ClaimResult *r){
    if(in->n < 2) return -1;
    int proj = in->v[0], meas = in->v[1];
    int ext = (in->n >= 3) ? in->v[2] : 0;
    long d = residual_estacao(proj, meas);
    r->forward = proj;
    r->reverse = meas;
    r->residual = d;
    r->closed = estacao_fecha(proj, meas, ext);
    r->mutation = ext ? 0 : 1;  /* réu escreve referência → BROKEN */
    r->klass = 0;
    return 0;
}

/* Banco: documento de implantação — emit ∘ parse = id (ob:banco). */
static int exec_banco(const ClaimInput *in, ClaimResult *r){
    if(!in || in->n < 2) return -1;
    int p = in->v[0], m = in->v[1];
    int ext = (in->n >= 3) ? in->v[2] : 1;
    char doc[IMPLANTE_MAX];
    if(implante_emit(p, m, ext, doc, IMPLANTE_MAX) < 0) return -1;
    int pp = 0, mm = 0, ee = 0;
    if(implante_parse(doc, &pp, &mm, &ee) != 0) return -1;
    int same = (pp == p && mm == m && ee == ext);
    r->forward = p;
    r->reverse = m;
    r->residual = same ? 0 : 1;
    r->closed = same;
    /* mutação alter_doc: mudar P no texto quebra a volta */
    char mut[IMPLANTE_MAX];
    implante_emit(p + 1, m, ext, mut, IMPLANTE_MAX);
    r->mutation = (strcmp(mut, doc) != 0) ? 1 : 0;
    r->klass = 0;
    return 0;
}

/* Cristal: v[0]=n fonte, v[1]=n reconstruído, v[2]=mismatches byte a byte,
 * v[3]=external (1 = medidor tests/cristal_volta.js correu; 0 = réu afirmou).
 * STEP project_latex → BACK reconstruct → MEASURE byte_compare. 0∧0 não fecha. */
static int exec_cristal(const ClaimInput *in, ClaimResult *r){
    if(!in || in->n < 3) return -1;
    int nf = in->v[0], nr = in->v[1], mm = in->v[2];
    int ext = (in->n >= 4) ? in->v[3] : 0;
    long d = (long)nf - (long)nr;
    if(d < 0) d = -d;
    r->forward = nf;
    r->reverse = nr;
    r->residual = d + (mm > 0 ? mm : 0);
    r->closed = (ext != 0) && (nf > 0) && (r->residual == 0);
    r->mutation = (nr > 0) ? 1 : 0;   /* alter_projection denuncia se há projeção */
    r->klass = 0;
    return 0;
}

/* Pátria: v[0]=local (corpo), v[1]=live (HTTP), v[2]=external (fetch, não réu). */
static int exec_deploy(const ClaimInput *in, ClaimResult *r){
    if(!in || in->n < 2) return -1;
    int loc = in->v[0], live = in->v[1];
    int ext = (in->n >= 3) ? in->v[2] : 0;
    r->forward = loc;
    r->reverse = live;
    r->residual = residual_patria(loc, live);
    r->closed = patria_fecha(loc, live, ext);
    r->mutation = loc ? 1 : 0;  /* drop_pipeline denuncia se o corpo tinha o .tex */
    r->klass = 0;
    return 0;
}

int claim_execute(const Claim *c, const ClaimInput *in, ClaimResult *out){
    if(!c || !out) return -1;
    memset(out, 0, sizeof *out);
    out->mutation = 2;
    out->klass = claim_class_of(c->classify);
    /* Fronteiras ANTES das ferramentas — objectos Interface* contêm GUT/Why/PDCA */
    if(strcmp(c->name, "FrontierGUT5W2H") == 0 || strstr(c->object, "InterfaceGUT")){
        if(!in) return -1;
        return exec_front_gut(in, out);
    }
    if(strcmp(c->name, "FrontierWhyIshikawa") == 0 || strstr(c->object, "InterfaceWhy")){
        if(!in) return -1;
        return exec_front_why(in, out);
    }
    if(strcmp(c->name, "FrontierPDCAVSM") == 0 || strstr(c->object, "InterfacePDCA")){
        if(!in) return -1;
        return exec_front_pdca(in, out);
    }
    if(strcmp(c->name, "StationResidual") == 0 || strstr(c->object, "Station")){
        if(!in) return -1;
        return exec_estacao(in, out);
    }
    if(strcmp(c->name, "BancoVolta") == 0 || strcmp(c->object, "Bank") == 0){
        if(!in) return -1;
        return exec_banco(in, out);
    }
    if(strcmp(c->name, "DeployPatria") == 0 || strcmp(c->object, "Publication") == 0){
        if(!in) return -1;
        return exec_deploy(in, out);
    }
    if(strcmp(c->name, "CristalVolta") == 0 || strcmp(c->object, "Cristal") == 0){
        if(!in) return -1;
        return exec_cristal(in, out);
    }
    if(strcmp(c->name, "ParetoClosure") == 0 || strstr(c->object, "Pareto")){
        if(!in) return -1;
        return exec_pareto(in, out);
    }
    if(strcmp(c->name, "WhyFixedPoint") == 0 || strcmp(c->object, "WhyChain") == 0){
        if(!in) return -1;
        return exec_why(in, out);
    }
    if(strcmp(c->name, "KanbanThree") == 0 || strstr(c->object, "Kanban")){
        if(!in) return -1;
        return exec_kanban(in, out);
    }
    if(strcmp(c->name, "PipelineClosure") == 0 || strstr(c->object, "Pipeline")){
        return exec_pipeline(c, out);
    }
    if(strcmp(c->name, "GUTProduct") == 0 || strcmp(c->object, "GUTMatrix") == 0){
        if(!in) return -1;
        return exec_gut(in, out);
    }
    if(strcmp(c->name, "PDCACycle") == 0 || strcmp(c->object, "PDCA") == 0){
        if(!in) return -1;
        return exec_pdca(in, out);
    }
    if(strcmp(c->name, "FiveW2H") == 0 || strstr(c->object, "ActionPlan")){
        if(!in) return -1;
        return exec_fivew2h(in, out);
    }
    if(strcmp(c->name, "IshikawaFree") == 0 || strstr(c->object, "CauseMap")){
        if(!in) return -1;
        return exec_ishikawa(in, out);
    }
    if(strcmp(c->name, "VSMDual") == 0 || strstr(c->object, "ValueStream")){
        if(!in) return -1;
        return exec_vsm(in, out);
    }
    if(strcmp(c->name, "FluxogramaDual") == 0 || strstr(c->object, "ProcessMap")){
        if(!in) return -1;
        return exec_fluxograma(in, out);
    }
    return -1;
}
