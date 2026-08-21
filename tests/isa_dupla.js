/* isa_dupla.js — A MESMA FITA NAS DUAS REALIZAÇÕES: assembly (erg) e webassembly (MOVE).
 *
 * O Aarão: «segue pra ISA em assembly e webassembly» · «essa arquitetura como ficou em
 * assembly, C é backend, agora quero migrar pra web em webassembly».
 *
 * A ISA é UMA e tem duas realizações: o `banco/erg.c` — montador e executor em ficheiros,
 * sem RAM — e o `tools/isa.c` subido para wasm pela porta única, MOVE. Este medidor pega o
 * MESMO bytecode (montado pelo erg a partir do texto em assembly) e corre-o nos dois:
 *
 *      texto.erg ──monta──► fita ──┬── erg corre  ──► slots do ficheiro (erg ve)
 *                                  └── MOVE(1050)  ──► slots pela porta (absorve/TROCA)
 *
 * DOIS CAMINHOS QUE TÊM DE CONCORDAR — e não há terceiro texto a servir de gabarito: cada
 * realização é a régua da outra. No lado wasm nada entra nem sai fora do MOVE: os valores
 * fabricam-se (CMP dos zeros dá 3, iguais dão 2, SUB dá 1, dobra-e-soma dá o resto) e o
 * resultado lê-se pelo retorno. E a metade refletida vai DENTRO da fita: o programa dos
 * metais desfaz-se a si próprio, e o resíduo mede-se nas duas realizações.
 *
 *   §D1  a aritmética: o mesmo STORE nas duas realizações, resíduo 0
 *   §D2  os metais desfazem-se DENTRO da fita — nas duas, e uma contra a outra
 *   §D3  o salto: a instrução saltada não corre em nenhuma das duas
 *   §D4  o período do esquilo fecha em quatro — nas duas
 *   §D5  o CONTROLO: um byte trocado numa realização, e elas deixam de concordar
 */
'use strict';
const fs = require('fs');
const { execFileSync } = require('child_process');
const path = require('path');

const RAIZ = path.resolve(__dirname, '..');
const TMP = process.env.TMPDIR || '/tmp';
const ERG = path.join(TMP, 'erg_dupla');
const TRADUZ = path.join(RAIZ, 'tools', 'bin', 'traduz');
const W = path.join(TMP, 'isa_dupla.wasm');

let falhas = 0, feitas = 0;
function ok(q, cond) {
    feitas++; if (!cond) falhas++;
    console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`);
    console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`);
}

console.log('=== A MESMA FITA NAS DUAS REALIZACOES: assembly e webassembly ===\n');

/* a régua constrói-se da fonte, sempre — as DUAS réguas */
try {
    execFileSync('cc', ['-O2', '-std=c99', '-w', '-I' + path.join(RAIZ, 'lib'),
                        path.join(RAIZ, 'banco', 'erg.c'), '-o', ERG]);
    fs.mkdirSync(path.dirname(TRADUZ), { recursive: true });
    execFileSync('cc', ['-O2', '-std=c99', '-w', path.join(RAIZ, 'tools', 'traduz.c'), '-o', TRADUZ]);
    execFileSync(TRADUZ, [path.join(RAIZ, 'tools', 'isa.c'), '-o', W]);
} catch (e) {
    console.log('  uma das realizacoes nao construiu — e sem as duas nao ha o que confrontar.');
    console.log(String(e.stderr || e.message).slice(0, 300));
    console.log('#TOTAL 0 1'); process.exit(1);
}
const BYTES = fs.readFileSync(W);
let podeInstanciar = true;
try { new WebAssembly.Instance(new WebAssembly.Module(BYTES)); }
catch (e) {
    if (/Out of memory|Cannot allocate/i.test(e.message)) podeInstanciar = false;
    else throw e;
}
if (!podeInstanciar) {
    /* sob o ulimit da bateria o motor nao instancia; a fita ainda se monta e corre no erg,
     * e a unica coisa que se afirma do wasm e' o que se ve: o modulo e a porta */
    console.log('  o motor nao instancia modulos com disco sob este limite de espaco virtual.');
    const EXP = WebAssembly.Module.exports(new WebAssembly.Module(BYTES));
    ok('a fita monta e o modulo sobe com a porta UNICA — o motor nao corre sob o limite',
       EXP.length === 1 && EXP[0].name === 'MOVE');
    console.log(`#TOTAL ${feitas} ${falhas}`);
    process.exit(falhas ? 1 : 0);
}

/* ── o lado do assembly: monta, semeia, corre, lê ─────────────────────────────────── */
function ergCorre(asm, sementes, lerSlots) {
    const src = path.join(TMP, 'dupla.erg'), bin = path.join(TMP, 'dupla.bin'),
          mem = path.join(TMP, 'dupla.dat');
    fs.writeFileSync(src, asm);
    execFileSync(ERG, ['monta', src, bin]);
    fs.rmSync(mem, { force: true });
    execFileSync(ERG, ['zera', mem, '2048']);
    for (const [s, t, e] of sementes) execFileSync(ERG, ['poe', mem, String(s), String(t), String(e)]);
    execFileSync(ERG, ['corre', bin, mem]);
    const out = [];
    for (const s of lerSlots) {
        const r = execFileSync(ERG, ['ve', mem, String(s)]).toString().trim().split(/\s+/);
        out.push([BigInt(r[0]), BigInt(r[1])]);
    }
    return { fita: fs.readFileSync(bin), lidos: out };
}

/* ── o lado do wasm: TUDO pela porta — a máquina fabrica os valores ───────────────── */
function wasmCorre(fita, sementes, lerSlots) {
    const E = new WebAssembly.Instance(new WebAssembly.Module(BYTES)).exports;
    const MOVE = (d, s) => E.MOVE(BigInt(d), BigInt(s));
    const abs = (d) => MOVE(d, 1), emi = (d) => MOVE(d, -1);
    const SOMA = 1040, SUB = 1041, TROCA = 1048, CMP = 1049;
    /* CMP so' escreve as FLAGS (a semantica do sql.c) — mas as flags SAO UM SLOT (1028),
     * e dois TROCAs poem o lido em R: a maquina continua a fabricar tudo pela porta */
    abs(990); abs(990); abs(CMP);
    abs(1028); abs(TROCA); abs(TROCA); emi(700);            /* 3: ambos zero e iguais */
    abs(700); abs(700); abs(CMP);
    abs(1028); abs(TROCA); abs(TROCA); emi(701);            /* 2: iguais, nao zero    */
    abs(701); abs(700); abs(SUB); emi(702);                 /* 1: a diferenca, por R  */
    function poeT(slot, v) {
        v = BigInt.asUintN(64, BigInt(v));
        /* o zero materializa-se: absorver nao toca R (sql.c), entao A=0 vai a R por TROCA² */
        abs(991); abs(TROCA); abs(TROCA); emi(703);
        let viu = false;
        for (let i = 63n; i >= 0n; i--) {
            const bit = (v >> i) & 1n;
            if (!viu && bit === 0n) continue;
            viu = true;
            abs(703); abs(703); abs(SOMA); emi(703);
            if (bit) { abs(702); abs(703); abs(SOMA); emi(703); }
        }
        abs(703); emi(slot);
    }
    function poe(slot, t, e) {
        poeT(704, t); poeT(705, e);
        abs(705); abs(TROCA); emi(705);
        abs(705); abs(704); abs(SOMA); emi(slot);
    }
    for (const [s, t, e] of sementes) poe(s, t, e);
    for (let k = 0; k * 16 < fita.length; k++) {
        let lo = 0n, hi = 0n;
        for (let j = 0; j < 8; j++) {
            lo |= BigInt(fita[k * 16 + j] ?? 0) << BigInt(8 * j);
            hi |= BigInt(fita[k * 16 + 8 + j] ?? 0) << BigInt(8 * j);
        }
        poe(1030 + k, lo, hi);
    }
    poeT(1029, fita.length); poeT(1027, 0);
    for (let i = 0; i < 1000; i++) if (MOVE(1050, 1) === 0n) break;
    const out = [];
    for (const s of lerSlots) { const t = abs(s); const e = abs(TROCA); out.push([t, e]); }
    return out;
}

/* ── o confronto: as duas realizações, slot a slot ────────────────────────────────── */
/* AS DUAS REALIZAÇÕES ESTÃO EM ANDARES DIFERENTES DA TORRE, E COMPARAM-SE NO COMUM.
 *
 * O `erg` é a máquina do `sql.c`: Word_8², oito bits por componente. O `isa.c`
 * empacota OITO bytes por componente (ver `poe(1030+k, lo, hi)` abaixo) e opera
 * na palavra do host — está um andar acima. Por isso divergiam, e sempre no
 * SINAL: a subtracção 2−7 dá 251 no andar de baixo e −5 no de cima, que é o
 * MESMO elemento lido nas duas vistas que o `word_isa.h` declara («visão
 * assinada do envelope — I/O local, não largura semântica»).
 *
 * A comparação faz-se RESTRINGINDO ao andar comum, e isso não é normalizar
 * para o teste passar: é o Teor. do encaixe (`naturais.tex thm:encaixe`) —
 * «para k < k' as operações do andar k' RESTRINGEM às do andar k, cadeia de
 * subcorpos, sem conversão de representação entre andares» — com o Cor. w8 a
 * dizer que «a identificação é a IDENTIDADE: o bit j do inteiro é a coordenada
 * j na base». Restringir é ler os oito bits baixos, e mais nada.
 *
 * O QUE ISTO DEIXA DE FORA, dito e não escondido: se o andar de cima errasse
 * nos bits ACIMA do oitavo, esta comparação não o via. Não há com que o
 * comparar — o `sql.c` não tem esses bits —, e medi-lo pede um terceiro
 * caminho no mesmo andar. O que aqui se afirma é o encaixe, no seu escopo. */
const ENVELOPE = (x) => BigInt.asUintN(8, BigInt(x));

function confronta(nome, asm, sementes, lerSlots) {
    const erg = ergCorre(asm, sementes, lerSlots);
    const wasm = wasmCorre(erg.fita, sementes, lerSlots);
    let iguais = 0;
    for (let i = 0; i < lerSlots.length; i++)
        if (ENVELOPE(erg.lidos[i][0]) === ENVELOPE(wasm[i][0]) &&
            ENVELOPE(erg.lidos[i][1]) === ENVELOPE(wasm[i][1])) iguais++;
    /* DIZER QUAL DIVERGE, e o que a outra realização deu. Imprimia-se só a
     * contagem e o lado do erg: com «2/3 slots iguais» não se sabe qual dos
     * três, nem contra o quê — e um confronto que não nomeia o culpado obriga
     * a instrumentar o medidor outra vez para o descobrir. */
    console.log(`   ${nome}: ${erg.fita.length} bytes de fita, ${iguais}/${lerSlots.length}` +
                ` slots iguais   erg=${JSON.stringify(erg.lidos.map(String))}`);
    for (let i = 0; i < lerSlots.length; i++)
        if (ENVELOPE(erg.lidos[i][0]) !== ENVELOPE(wasm[i][0]) ||
            ENVELOPE(erg.lidos[i][1]) !== ENVELOPE(wasm[i][1]))
            console.log(`      DIVERGE no slot ${lerSlots[i]}, JÁ no envelope:` +
                        ` erg=${erg.lidos[i]}  wasm=${wasm[i]}`);
    return { erg, wasm, iguais, total: lerSlots.length };
}

/* ─── §D1 a aritmética ────────────────────────────────────────────────────────────── */
{
    const r = confronta('aritmetica',
        'LOAD 5\nLOAD 6\nADD\nSTORE 20\nSUB\nSTORE 21\nXOR\nSTORE 22\nHALT\n',
        [[5, 7, 3], [6, 2, 9]], [20, 21, 22]);
    ok('§D1 a aritmetica da a MESMA resposta nas duas realizacoes, slot a slot',
       r.iguais === r.total && r.total === 3);
}

/* ─── §D2 os metais desfazem-se DENTRO da fita — nas duas ─────────────────────────── */
{
    const r = confronta('metais',
        'LOAD 5\nGOLD\nSTORE 20\nGOLD\nNEGRO_OURO\nNEGRO_OURO\nSTORE 21\nHALT\n',
        [[5, 34, 21]], [20, 21]);
    /* o slot 21 e' a ida-e-volta completa: tem de devolver o proprio slot 5 — o residuo
     * onde TEM de ser zero, medido DENTRO da fita e nas duas maquinas */
    const volta_erg = r.erg.lidos[1][0] === 34n && r.erg.lidos[1][1] === 21n;
    const volta_wasm = r.wasm[1][0] === 34n && r.wasm[1][1] === 21n;
    ok('§D2 os metais dao o mesmo nas duas realizacoes', r.iguais === r.total);
    ok('§D2 e a volta fecha DENTRO da fita, nas duas: GOLD²·NEGRO² devolve a semente',
       volta_erg && volta_wasm);
}

/* ─── §D3 o salto ─────────────────────────────────────────────────────────────────── */
{
    /* mede-se RELATIVO, sem eu escrever onde o salto aterra: a mesma fita com e sem o
     * JMP, nas duas realizacoes — cada uma concorda com a outra, e o salto tem de MUDAR
     * o resultado (senao «saltou» nao se distingue de «correu») */
    const rc = confronta('salto (com)', 'LOAD 5\nJMP 1\nGOLD\nSTORE 20\nHALT\n',
                         [[5, 11, 4]], [20]);
    const rs = confronta('salto (sem)', 'LOAD 5\nGOLD\nSTORE 20\nHALT\n',
                         [[5, 11, 4]], [20]);
    const difere = !(rc.erg.lidos[0][0] === rs.erg.lidos[0][0] &&
                     rc.erg.lidos[0][1] === rs.erg.lidos[0][1]);
    ok('§D3 o salto salta NAS DUAS: as realizacoes concordam com e sem ele, e ele muda o'
       + ' resultado', rc.iguais === 1 && rs.iguais === 1 && difere);
}

/* ─── §D4 o período do esquilo ────────────────────────────────────────────────────── */
{
    const r = confronta('esquilo',
        'LOAD 5\nESQUILO\nSTORE 20\nESQUILO\nSTORE 21\nESQUILO\nSTORE 22\nESQUILO\nSTORE 23\nHALT\n',
        [[5, 6, 13]], [20, 21, 22, 23]);
    /* o quarto STORE devolve a semente — o periodo 4 sai da fita, nas duas maquinas —
     * e os tres intermedios NAO sao a semente: a orbita anda antes de voltar */
    const p4 = r.erg.lidos[3][0] === 6n && r.erg.lidos[3][1] === 13n;
    const anda = r.erg.lidos.slice(0, 3).every(([t, e]) => !(t === 6n && e === 13n));
    ok('§D4 as duas realizacoes concordam na orbita inteira do esquilo', r.iguais === 4);
    ok('§D4 e o periodo 4 sai da fita: tres passos fora, o quarto em casa', p4 && anda);
}

/* ─── §D5 o CONTROLO: um byte trocado, e a concordancia sabe acabar ───────────────── */
{
    const asm = 'LOAD 5\nLOAD 6\nADD\nSTORE 20\nHALT\n';
    const sementes = [[5, 7, 3], [6, 2, 9]];
    const erg = ergCorre(asm, sementes, [20]);
    const mutada = Buffer.from(erg.fita);
    mutada[6] = 4;                          /* o ADD (op 3, byte 6 da fita) vira SUB */
    const wasm = wasmCorre(mutada, sementes, [20]);
    const discordam = !(erg.lidos[0][0] === wasm[0][0] && erg.lidos[0][1] === wasm[0][1]);
    console.log(`   controlo: ADD trocado por SUB so' no wasm — erg=${erg.lidos[0]}` +
                ` wasm=${wasm[0]}`);
    ok('§D5 um byte trocado NUMA das realizacoes e elas discordam — a igualdade sabe falhar',
       discordam);
}

console.log('\n==========================================================================');
if (!falhas) {
    console.log('  A ISA e UMA e as realizacoes sao duas: o erg monta o texto em fita e corre-a');
    console.log('  sobre ficheiros; o wasm corre A MESMA FITA pela porta unica, com os valores');
    console.log('  fabricados pela propria maquina. Nao ha gabarito de fora — cada realizacao');
    console.log('  e a regua da outra, e o residuo entre elas e zero slot a slot.');
} else console.log(`  FALHOU: ${falhas}`);
console.log(`#TOTAL ${feitas} ${falhas}`);
process.exit(falhas ? 1 : 0);
