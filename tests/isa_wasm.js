/* isa_wasm.js — A ISA EM WEBASSEMBLY: A PORTA É UMA, E MEDE-SE PELA METADE REFLETIDA.
 *
 * O Aarão: «a única função que deve ter no ISA é o MOVE» · «não tem opcode inventado DISCO:
 * o MOVE corre aonde senão no disco?» · «lê papers/corpo_analitico.tex e aprende a medir
 * nesse sistema pela metade refletida».
 *
 * E o §sec:medir diz como: «A medida não compara: reverte, e lê o resíduo. [...] A medida
 * são as duas metades da mesma passagem: o resíduo onde TEM de ser zero diz que EXISTE; o
 * resíduo onde NÃO PODE ser zero diz que é ÚNICO.»
 *
 * Aqui isso é literal, e é a única forma de medir por uma porta de UMA função:
 *
 *   - a MÁQUINA FABRICA os valores: CMP dos zeros dá 3, dois iguais dão 2, SUB dá 1, e a
 *     órbita do GOLD gera o resto — nenhum vetor de teste é meu;
 *   - o REFLEXO desfaz a ida na mesma corrida (GOLD·NEGRO, ESQUILO⁴, TROCA², emitir·absorver);
 *   - o RESÍDUO sai PELA PORTA: o SUB da própria máquina contra o guardado, e o retorno do
 *     MOVE é o total — zero se fechou. O `e` lê-se pelo TROCA, pela mesma porta.
 *
 * Não há vista do disco, não há BigInt meu a entrar, não há número esperado escrito — só
 * MOVE(destino, sentido), que é toda a porta que o módulo tem.
 *
 *   §I0  a porta é UMA — e cada opcode dá o MESMO que o MOVE direto ao destino (reflexo)
 *   §I1  emitir·absorver são inversos — e o slot nunca escrito NÃO devolve (a outra metade)
 *   §I2  o salto é o MESMO MOVE com o pc por destino
 *   §I3  os metais desfazem-se — e sem a volta o resíduo NÃO é zero
 *   §I4  os períodos 4 e 2 saem da máquina; o GOLD não fecha
 *   §I5  a ULA contra o motor de baixo (aqui compara-se, e diz-se)
 */
'use strict';
const fs = require('fs');
const { execFileSync } = require('child_process');
const path = require('path');

const RAIZ = path.resolve(__dirname, '..');
const TRADUZ = path.join(RAIZ, 'tools', 'bin', 'traduz');
const FONTE = path.join(RAIZ, 'tools', 'isa.c');
const TMP = process.env.TMPDIR || '/tmp';

let falhas = 0, feitas = 0;
function ok(q, cond) {
    feitas++; if (!cond) falhas++;
    console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`);
    console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`);
}

console.log('=== A ISA EM WEBASSEMBLY: a porta e UMA, e mede-se pela metade refletida ===\n');

/* a régua constrói-se da fonte, sempre */
const w = path.join(TMP, 'isa.wasm');
try {
    fs.mkdirSync(path.dirname(TRADUZ), { recursive: true });
    execFileSync('cc', ['-O2', '-std=c99', '-w', path.join(RAIZ, 'tools', 'traduz.c'), '-o', TRADUZ]);
    execFileSync(TRADUZ, [FONTE, '-o', w]);
} catch (e) {
    console.log('  a ISA nao subiu — e sem ela nao ha nada a medir.');
    console.log(String(e.stderr || e.message).slice(0, 300));
    console.log('#TOTAL 0 1'); process.exit(1);
}

const bytes = fs.readFileSync(w);
const Modulo = new WebAssembly.Module(bytes);
const EXPORTS = WebAssembly.Module.exports(Modulo);
let E;
try { E = new WebAssembly.Instance(Modulo).exports; }
catch (e) {
    if (/Out of memory|Cannot allocate/i.test(e.message)) {
        console.log('  o motor nao instancia modulos com disco sob este limite de espaco virtual.');
        console.log(`  a ISA subiu: ${bytes.length} bytes, ${EXPORTS.length} exports.`);
        ok('a ISA sobe e a porta e UMA: MOVE, e mais nada — o motor nao corre sob o limite',
           EXPORTS.length === 1 && EXPORTS[0].name === 'MOVE' && EXPORTS[0].kind === 'function');
        console.log(`#TOTAL ${feitas} ${falhas}`);
        process.exit(falhas ? 1 : 0);
    }
    throw e;
}

/* ── TUDO PELA PORTA. Absorver devolve o total; o `e` lê-se pelo TROCA. ───────────── */
const MOVE = (d, s) => E.MOVE(BigInt(d), BigInt(s));
const abs = (d) => MOVE(d, 1);          /* absorve: slot -> A, e devolve o total */
const emi = (d) => MOVE(d, -1);         /* emite: R -> slot */
const SOMA = 1040, SUB = 1041, GOLD = 1045, NEGRO = 1046, ESQUILO = 1047, TROCA = 1048, CMP = 1049;
/* o resíduo A−(slot s), lido pela porta: [t, e] — o SUB é o da máquina, não meu. A ULA só
 * escreve R (sql.c), então o e do resíduo materializa-se: R vai a um slot, o slot a A, e o
 * TROCA devolve-o pelo retorno. */
function residuo(s) {
    abs(s); const t = abs(SUB);
    emi(748); abs(748); const e = abs(TROCA);
    return [t, e];
}
/* a diferença entre DOIS slots, pela mesma via */
function resDif(s1, s2) {
    abs(s2); abs(s1); const t = abs(SUB);
    emi(748); abs(748); const e = abs(TROCA);
    return [t, e];
}

/* ── A MÁQUINA FABRICA AS SUAS CONSTANTES — nenhum valor entra de fora ────────────── */
/* slots 700+ são o estaleiro; 990+ nunca se emitem (são o «nunca escrito») */
/* CMP so' escreve as FLAGS — semantica do sql.c, o gabarito que o isa_dupla impos — mas as
 * flags SAO UM SLOT (1028), e dois TROCAs materializam o lido em R: tudo pela porta */
abs(990); abs(990); abs(CMP);
abs(1028); abs(TROCA); abs(TROCA); emi(700);         /* 3: ambos zero e iguais */
abs(700); abs(700); abs(CMP);
abs(1028); abs(TROCA); abs(TROCA); emi(701);         /* 2: iguais, nao zero    */
abs(701); abs(700); abs(SUB); emi(702);              /* 1: a diferenca, por R  */
const TRES = abs(700), DOIS = abs(701), UM = abs(702);
console.log(`   a maquina fabricou: ${TRES}, ${DOIS}, ${UM} — CMP dos zeros, dois iguais, SUB`);

/* (v,0) no slot, por dobra-e-soma do 1 que a máquina fez — tudo pela porta */
function poeT(slot, v) {
    v = BigInt.asUintN(64, BigInt(v));
    abs(991); abs(TROCA); abs(TROCA); emi(703);      /* acc = 0: absorver nao toca R,
                                                      * entao o zero vai a R por TROCA² */
    let viu = false;
    for (let i = 63n; i >= 0n; i--) {
        const bit = (v >> i) & 1n;
        if (!viu && bit === 0n) continue;
        viu = true;
        abs(703); abs(703); abs(SOMA); emi(703);     /* acc += acc */
        if (bit) { abs(702); abs(703); abs(SOMA); emi(703); }   /* acc += 1 */
    }
    abs(703); emi(slot);
}
/* o programa: bytes empacotados nos slots 1030+, o tamanho em 1029, o pc a zero */
function gravaProg(prog) {
    for (let k = 0; k * 16 < prog.length; k++) {
        let lo = 0n, hi = 0n;
        for (let j = 0; j < 8; j++) {
            lo |= BigInt(prog[k * 16 + j] ?? 0) << BigInt(8 * j);
            hi |= BigInt(prog[k * 16 + 8 + j] ?? 0) << BigInt(8 * j);
        }
        poeT(704, lo); poeT(705, hi);
        abs(705); abs(TROCA); emi(705);              /* 705 = (0,hi) */
        abs(705); abs(704); abs(SOMA); emi(1030 + k);/* (lo,hi) no slot do programa */
    }
    poeT(1029, prog.length); poeT(1027, 0);
}
function corre(max) { for (let i = 0; i < max; i++) if (MOVE(1050, 1) === 0n) break; }
const opGOLD = 8, opHALT = 0, opJMP = 10;

/* ─── §I0 a porta é UMA — e o opcode é o destino, pelo reflexo ────────────────────── */
{
    console.log(`   a porta do modulo: ${EXPORTS.map(x => `${x.name}(${x.kind})`).join(', ')}`);
    ok('§I0 a porta e UMA: o modulo exporta MOVE e mais NADA — nem funcao, nem disco',
       EXPORTS.length === 1 && EXPORTS[0].name === 'MOVE' && EXPORTS[0].kind === 'function');

    /* o opcode 8 do programa e o destino 1045 da porta têm de dar o MESMO: corre-se um,
     * guarda-se, corre-se o outro, e o SUB da máquina lê o resíduo — zero se são um. */
    let iguais = 0, casos = 0;
    for (const [op, dest] of [[3, SOMA], [4, SUB], [5, 1042], [6, 1043], [7, 1044],
                              [8, GOLD], [15, NEGRO], [16, ESQUILO], [17, TROCA], [9, CMP]]) {
        gravaProg([op, opHALT]);
        /* CMP arma-se com IGUAIS (f=2); os outros com (3,2) — depois do grava, sempre */
        if (dest === CMP) { abs(700); abs(700); } else { abs(701); abs(700); }
        corre(5);
        casos++;
        if (dest === CMP) {
            /* CMP so' escreve as flags — compara-se o slot 1028 dos dois caminhos, e
             * exige-se f≠0: um zero igual dos dois lados nao provava conta nenhuma */
            const f1 = abs(1028);
            abs(700); abs(700);
            MOVE(dest, 1);
            const f2 = abs(1028);
            if (f1 === f2 && f1 !== 0n) iguais++;
        } else {
            emi(730);                                /* o que o OPCODE deixou em R */
            abs(701); abs(700);                      /* mesmos operandos outra vez */
            MOVE(dest, 1);                           /* o DESTINO direto */
            emi(731);                                /* o que o DESTINO deixou em R */
            const [rt, re] = resDif(730, 731);
            if (rt === 0n && re === 0n) iguais++;
        }
    }
    console.log(`   ${casos} opcodes contra os seus destinos: ${iguais} com residuo 0`);
    ok('§I0 o opcode E o destino: o residuo entre os dois caminhos e 0 pela propria maquina',
       casos === 10 && iguais === casos);
}

/* ─── §I1 emitir·absorver são inversos — e a metade que NÃO PODE ser zero ─────────── */
{
    let fecha = 0, casos = 0, reflete = 0;
    abs(700); abs(GOLD);                             /* a órbita arranca do 3 */
    for (let n = 0; n < 60; n++) {
        const s = [0, 7, 63, 1023][n % 4];
        emi(740);                                    /* guarda o estado da órbita */
        emi(s);                                      /* emite para o slot da vez */
        abs(s);                                      /* absorve de volta */
        const [rt, re] = residuo(740);               /* contra o guardado: TEM de ser 0 */
        casos++;
        if (rt === 0n && re === 0n) fecha++;
        /* a outra metade: um slot NUNCA escrito no lugar do reflexo — NÃO PODE ser 0 */
        abs(992);
        const [nt, ne] = residuo(740);
        if (nt !== 0n || ne !== 0n) reflete++;
        abs(740); abs(GOLD);                         /* a órbita segue */
    }
    console.log(`   emitir·absorver ao longo da orbita: ${fecha}/${casos} com residuo 0;` +
                ` o nunca-escrito falha ${reflete}/${casos}`);
    ok('§I1 emitir e absorver sao inversos: residuo 0 onde TEM de ser, ao longo da orbita',
       casos === 60 && fecha === casos);
    ok('§I1 e o slot nunca escrito NAO devolve: o residuo onde NAO PODE ser zero, nao e',
       reflete === casos);
}

/* ─── §I2 o salto é o mesmo MOVE, com o pc por destino ────────────────────────────── */
{
    abs(700); abs(GOLD); emi(741);                   /* um estado da órbita, guardado */
    gravaProg([opJMP, 1, opGOLD, opHALT, opHALT]);   /* salta por cima do GOLD */
    abs(741); corre(10);
    const [st_, se_] = residuo(741);                 /* saltou: nada mudou -> 0 */
    gravaProg([opGOLD, opHALT]);
    abs(741); corre(10);
    const [ct, ce] = residuo(741);                   /* correu: mudou -> nao-zero */
    console.log(`   com salto o residuo e (${st_},${se_}); sem salto e (${ct},${ce})`);
    ok('§I2 o salto e o MESMO MOVE com o pc por destino: salta e o residuo e 0; sem ele nao',
       st_ === 0n && se_ === 0n && (ct !== 0n || ce !== 0n));
}

/* ─── §I3 os metais desfazem-se — e sem a volta, não ──────────────────────────────── */
{
    let fecha = 0, casos = 0, semvolta = 0;
    abs(700); abs(GOLD); abs(GOLD);
    for (let n = 0; n < 60; n++) {
        emi(742);
        abs(GOLD); abs(NEGRO);                       /* a ida e o reflexo */
        const [rt, re] = residuo(742);
        casos++;
        if (rt === 0n && re === 0n) fecha++;
        abs(742); abs(GOLD);                         /* só a ida — NÃO PODE fechar */
        const [nt, ne] = residuo(742);
        if (nt !== 0n || ne !== 0n) semvolta++;
        abs(742); abs(GOLD);                         /* a órbita segue */
    }
    console.log(`   GOLD·NEGRO na orbita: ${fecha}/${casos} fecham; so GOLD: ${semvolta}/${casos} nao fecham`);
    ok('§I3 os metais desfazem-se com a volta INTEIRA: residuo 0 pela propria maquina',
       casos === 60 && fecha === casos);
    ok('§I3 e tirada a reversao o residuo NAO e zero — a volta e que fazia fechar',
       semvolta === casos);
}

/* ─── §I4 os períodos saem da máquina ─────────────────────────────────────────────── */
{
    abs(700); abs(GOLD); abs(GOLD); abs(GOLD); emi(743);
    function periodo(dest) {
        for (let k = 1; k <= 8; k++) {
            abs(743);
            for (let j = 0; j < k; j++) abs(dest);
            const [rt, re] = residuo(743);
            if (rt === 0n && re === 0n) return k;
        }
        return 0;
    }
    const pe = periodo(ESQUILO), pt = periodo(TROCA), pg = periodo(GOLD);
    console.log(`   o periodo de ESQUILO: ${pe}   o de TROCA: ${pt}   o de GOLD: ${pg || 'nao fecha'}`);
    ok('§I4 ESQUILO fecha em QUATRO — e o quatro sai da maquina, nao de mim', pe === 4);
    ok('§I4 TROCA fecha em DOIS — a involucao, det −1', pt === 2);
    ok('§I4 e o GOLD NAO fecha em oito passos — o gato estica, ordem infinita', pg === 0);
}

/* ─── §I5 a ULA contra o motor de baixo — aqui compara-se, e diz-se ───────────────── */
{
    /* os operandos vêm da órbita e LEEM-SE pela porta (o retorno do absorver); o outro
     * lado é o BigInt do motor, que não é meu. */
    let mau = 0, casos = 0;
    abs(700); emi(750); abs(GOLD); emi(751);
    for (let n = 0; n < 20; n++) {
        const a = abs(750), b = abs(751);
        for (const [dest, f] of [[SOMA, (x, y) => x + y], [SUB, (x, y) => x - y],
                                 [1042, (x, y) => x & y], [1043, (x, y) => x | y],
                                 [1044, (x, y) => x ^ y]]) {
            abs(751); abs(750);                      /* A=(a), B=(b) */
            const r = MOVE(dest, 1);
            casos++;
            if (r !== BigInt.asIntN(64, f(a, b))) mau++;
        }
        abs(751); abs(GOLD); emi(750);               /* os operandos andam com a órbita */
        abs(750); abs(GOLD); emi(751);
    }
    console.log(`   a ULA sobre NAND contra o motor, ${casos} operacoes: ${mau} diferencas`);
    ok('§I5 a ULA sai do NAND e da o mesmo que os operadores de baixo — residuo 0 INTEIRO',
       casos === 100 && mau === 0);
}

console.log('\n==========================================================================');
if (!falhas) {
    console.log('  A porta e UMA — MOVE(destino, sentido) — e chegou para medir tudo: a maquina');
    console.log('  fabrica os valores (CMP dos zeros, dois iguais, SUB), a orbita gera o resto,');
    console.log('  o reflexo desfaz a ida na mesma corrida e o residuo sai pela propria porta.');
    console.log('');
    console.log('  E cada seccao tem as DUAS metades do corpo_analitico: o residuo onde TEM de');
    console.log('  ser zero diz que existe; o residuo onde NAO PODE ser zero diz que e unico.');
} else console.log(`  FALHOU: ${falhas}`);
console.log(`#TOTAL ${feitas} ${falhas}`);
process.exit(falhas ? 1 : 0);
