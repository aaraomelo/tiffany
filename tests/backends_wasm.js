/* backends_wasm.js — As linguagens-backend sobem pelo traduz; mede-se pela metade refletida.
 *
 * §W0  manifesto: cada fonte existe e sobe
 * §W1  pontes: escapar / decidir / rotular batem com o gabarito
 * §W2  claim: Pareto/Why/Kanban fecham R (CLOSE) e mutação denuncia
 * §W3  isa: porta única MOVE
 * §W4  latex: compor† = descompor (involução)
 * §W5  sql: compilar† = descompilar (interface padrão)
 * §W6  isabelle x⊕x=0
 * §W7  control: RETAIN/MOVE/RETRACT na arena (8 eixos)
 * §W8  bash: escreve/le na arena (protocolo; pleno em tests/bash_pleno.c)
 * §W9  dom: html compor† + css aplicar + js escapar (front via banco)
 * §W10 powershell + node: protocolo na arena (pleno em tests/*_pleno.c)
 * §W11 absorcao.move em cada língua; ptx/llvm/glsl/wasm MOVE na arena
 *
 *   node tests/backends_wasm.js
 */
import fs from 'node:fs'
import path from 'node:path'
import { execFileSync } from 'node:child_process'
import { fileURLToPath } from 'node:url'

const RAIZ = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..')
const OUT = path.join(RAIZ, 'assets', 'figuras', 'wasm')
const MAN = JSON.parse(fs.readFileSync(path.join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))
const BASE = MAN.nulo_disco || 8

let falhas = 0, feitas = 0;
function ok(q, cond) {
    feitas++; if (!cond) falhas++;
    console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`);
}

console.log('=== BACKENDS WASM: realizações pela mesma porta (traduz) ===\n');

function wasmPronto (L) {
    const w = path.join(OUT, L.wasm)
    return fs.existsSync(w) && fs.statSync(w).size > 32
}

try {
    const faltam = MAN.linguagens.filter((L) => !wasmPronto(L))
    if (faltam.length) {
        const cc = process.env.CC || 'gcc'
        const traduzBin = path.join(RAIZ, 'tools', 'bin', process.platform === 'win32' ? 'traduz.exe' : 'traduz')
        fs.mkdirSync(path.dirname(traduzBin), { recursive: true })
        execFileSync(cc, ['-O2', '-std=c99', '-w', path.join(RAIZ, 'tools', 'traduz.c'), '-o', traduzBin], { stdio: 'inherit' })
        execFileSync(process.execPath, [path.join(RAIZ, 'tools', 'sobe_backends_wasm.mjs')], { stdio: 'inherit' })
    }
} catch (e) {
    ok('sobe_backends_wasm corre', false);
    console.log('#TOTAL', feitas, falhas + 1);
    process.exit(1);
}

/* §W0 */
{
    let okN = 0;
    for (const L of MAN.linguagens) {
        const w = path.join(OUT, L.wasm);
        if (fs.existsSync(w) && fs.statSync(w).size > 32) okN++;
        else console.log('  falta', L.wasm);
    }
    ok(`§W0 manifesto: ${okN}/${MAN.linguagens.length} wasm no disco`, okN === MAN.linguagens.length);
}

async function load(nome) {
    const buf = fs.readFileSync(path.join(OUT, nome));
    const { instance } = await WebAssembly.instantiate(buf);
    return instance.exports;
}
function u8(ex) { return new Uint8Array(ex.DISCO.buffer); }
function i32(ex) { return new Int32Array(ex.DISCO.buffer, BASE); }

(async () => {
    const enc = new TextEncoder(), dec = new TextDecoder();

    /* §W1 pontes */
    {
        const e = await load('escapar.wasm');
        const m = u8(e);
        const t = 'A âncora: Hodge {$$';
        const b = enc.encode(t); m.set(b, BASE + 1024);
        const ln = e.escapar(1024, b.length, 8192);
        const out = dec.decode(m.slice(BASE + 8192, BASE + 8192 + ln));
        ok('§W1 escapar bate gabarito', out === 'A âncora: Hodge \\{\\$\\$');

        const d = await load('decidir.wasm');
        const a = i32(d);
        const nos = [{ p: 0, e: 0 }, { p: 4, e: 2 }, { p: 2, e: 1 }];
        const n = nos.length;
        for (let i = 0; i < n; i++) { a[i] = nos[i].p; a[n + i] = nos[i].e; }
        d.decidir(n);
        ok('§W1 decidir prof→cmd (4→3)', a[2 * n] === 0 && a[2 * n + 1] === 3 && a[3 * n + 1] === 2);

        const r = await load('rotular.wasm');
        const mr = u8(r);
        const id = enc.encode('2_semantica_formal');
        mr.set(id, BASE + 1024);
        const lnR = r.rotular(1024, id.length, 8192);
        const rot = dec.decode(mr.slice(BASE + 8192, BASE + 8192 + lnR));
        ok('§W1 rotular bate gabarito', rot === 'grafo:2semanticaformal');
    }

    /* §W2 claims */
    {
        const c = await load('claim.wasm');
        const a = i32(c);
        a[0] = 4; a[1] = 40; a[2] = 30; a[3] = 20; a[4] = 10;
        const closeP = c.claim_run(0);
        ok('§W2 Pareto CLOSE residual 0', closeP === 1 && a[2] === 0 && a[3] === 1);

        a[0] = 5; a[1] = 3;
        const closeW = c.claim_run(1);
        ok('§W2 Why ponto fixo (depth≥root)', closeW === 1 && a[0] === 0);

        a[0] = 16;
        const closeK = c.claim_run(2);
        ok('§W2 Kanban 3 estados sem colisão', closeK === 1 && a[0] === 0 && a[3] === 1);

        a[0] = 3; a[1] = 4; a[2] = 5;
        const closeG = c.claim_run(3);
        ok('§W2 GUT CLOSE score 60', closeG === 1 && a[0] === 60 && a[2] === 1);

        a[0] = 10; a[1] = 10; a[2] = 1;
        const closeD = c.claim_run(4);
        ok('§W2 PDCA CLOSE com alvo numerico', closeD === 1 && a[0] === 0 && a[2] === 1);

        a[0] = 1; a[1] = 7;
        const close5 = c.claim_run(5);
        ok('§W2 FiveW2H CLOSE com raiz', close5 === 1 && a[0] === 0);

        a[0] = 6; a[1] = 42; a[2] = 42;
        const closeI = c.claim_run(6);
        ok('§W2 Ishikawa CLOSE k livre (mut SURVIVED)', closeI === 1 && a[0] === 0 && a[2] === 0);

        a[0] = 1; a[1] = 1;
        const closeV = c.claim_run(7);
        ok('§W2 VSM CLOSE dual nomeado', closeV === 1 && a[0] === 0);

        a[0] = 5; a[1] = 5;
        const closeF = c.claim_run(8);
        ok('§W2 Fluxograma CLOSE nos pareados', closeF === 1 && a[0] === 0);

        a[0] = 4;
        a[1] = 60; a[2] = 48; a[3] = 30; a[4] = 12;
        a[5] = 0; a[6] = 1; a[7] = 2; a[8] = 3;
        const closeFG = c.claim_run(9);
        ok('§W2 Fronteira GUT/5W2H R=0', closeFG === 1 && a[0] === 0);

        a[0] = 7; a[1] = 3; a[2] = 1; a[3] = 7; a[4] = 2;
        const closeFW = c.claim_run(10);
        ok('§W2 Fronteira Why/Ishikawa hit', closeFW === 1);

        a[0] = 42; a[1] = 42;
        const closeFP = c.claim_run(11);
        ok('§W2 Fronteira PDCA/VSM eq', closeFP === 1 && a[0] === 0);

        a[0] = 65; a[1] = 65; a[2] = 1;
        const closeE = c.claim_run(12);
        ok('§W2 Estacao CLOSE volta MES', closeE === 1 && a[0] === 0 && a[2] === 0);

        a[0] = 65; a[1] = 40; a[2] = 1;
        const closeB = c.claim_run(13);
        ok('§W2 BancoVolta write=retrieve', closeB === 1 && a[0] === 0 && a[2] === 1);

        a[0] = 1; a[1] = 1; a[2] = 1;
        const closeDep = c.claim_run(14);
        ok('§W2 DeployPatria CLOSE local+live', closeDep === 1 && a[0] === 0);

        a[0] = 4286; a[1] = 4286; a[2] = 0; a[3] = 1;
        const closeCr = c.claim_run(15);
        ok('§W2 CristalVolta CLOSE fonte==reconstruido', closeCr === 1 && a[0] === 0);
        a[0] = 4286; a[1] = 4285; a[2] = 0; a[3] = 1;
        ok('§W2 CristalVolta conceito apagado REOPEN', c.claim_run(15) === 0 && a[0] === 1);
    }

    /* §W3 ISA */
    {
        const isa = await load('isa.wasm');
        const ex = WebAssembly.Module.exports(new WebAssembly.Module(fs.readFileSync(path.join(OUT, 'isa.wasm'))));
        const funs = ex.filter(x => x.kind === 'function');
        ok('§W3 ISA porta única MOVE', funs.length === 1 && funs[0].name === 'MOVE' && typeof isa.MOVE === 'function');
    }

    /* §W4 latex involução */
    {
        const L = await load('compor.wasm');
        const m = u8(L);
        const src = enc.encode('\\section{Oi}');
        m.set(src, BASE + 100);
        const n1 = L.latex_compor(100, src.length, 2000);
        const n2 = L.latex_descompor(2000, n1, 4000);
        const back = dec.decode(m.slice(BASE + 4000, BASE + 4000 + n2));
        ok('§W4 latex compor†=descompor', back === '\\section{Oi}');
    }

    /* §W5 sql involução (interface padrão) */
    {
        const S = await load('consultar.wasm');
        const m = u8(S);
        const src = enc.encode("INSERT TEXTO 'ouro'");
        m.set(src, BASE + 100);
        const n1 = S.sql_compilar(100, src.length, 2000);
        const n2 = S.sql_descompilar(2000, n1, 4000);
        const back = dec.decode(m.slice(BASE + 4000, BASE + 4000 + n2));
        ok('§W5 sql compilar†=descompilar (interface padrão)', back === "INSERT TEXTO 'ouro'");
    }

    /* isabelle residual */
    {
        const p = await load('provar.wasm');
        const a = i32(p);
        for (let i = 0; i < 8; i++) a[i] = (i + 1) * 17;
        const falhasXor = p.provar_xor_nulo(8);
        ok('§W6 isabelle x⊕x=0', falhasXor === 0);
    }

    /* §W8 bash — protocolo na arena (pleno = tests/bash_pleno.c) */
    {
        const B = await load('interpretar.wasm');
        const m = u8(B);
        ok('§W8 bash exports', typeof B.bash_move === 'function' &&
            typeof B.bash_escreve === 'function' &&
            typeof B.bash_le === 'function' && typeof B.bash_pronto === 'function');
        const cmd = enc.encode('echo ouro\n');
        m.set(cmd, BASE + 1024);
        const nw = B.bash_move(1024, cmd.length, 0, -1);
        ok('§W8 bash_move -1 aceita comando', nw === cmd.length);
        ok('§W8 bash_pendente', B.bash_pendente() === cmd.length);
        ok('§W8 bash_pronto inicia vazio', B.bash_pronto() === 0);
        ok('§W8 bash_corre export', typeof B.bash_corre === 'function');
        const gotB = B.bash_corre();
        ok('§W8 bash_corre semântica', gotB > 0);
        const nr = B.bash_move(8192, 32, 8192, +1);
        const got = dec.decode(m.slice(BASE + 8192, BASE + 8192 + nr));
        ok('§W8 bash_move +1 devolve stdout', got.indexOf('ouro') >= 0);
    }

    /* §W9 DOM — html + css + js (backends do front) */
    {
        const H = await load('html_compor.wasm');
        const mH = u8(H);
        const html = enc.encode('<p>oi</p>');
        mH.set(html, BASE + 100);
        const hn = H.html_move(100, html.length, 2000, -1);
        const hn2 = H.html_move(2000, hn, 4000, +1);
        const htmlBack = dec.decode(mH.slice(BASE + 4000, BASE + 4000 + hn2));
        ok('§W9 html_move ±1', htmlBack === '<p>oi</p>');

        const C = await load('css_aplicar.wasm');
        const mC = u8(C);
        const css = enc.encode('.x { color: red }\n.y { margin: 0 }');
        mC.set(css, BASE + 100);
        const cn = C.css_move(100, css.length, 2000, -1);
        const cssOut = dec.decode(mC.slice(BASE + 2000, BASE + 2000 + cn));
        ok('§W9 css_move -1 normaliza', cssOut === '.x { color: red }; .y { margin: 0 };');

        const J = await load('js_escapar.wasm');
        const mJ = u8(J);
        const js = enc.encode('a\n"b"');
        mJ.set(js, BASE + 100);
        const jn = J.js_move(100, js.length, 2000, -1);
        const jsOut = dec.decode(mJ.slice(BASE + 2000, BASE + 2000 + jn));
        ok('§W9 js_move -1', jsOut === 'a\\n\\"b\\"');
    }

    /* §W10 powershell + node — protocolo na arena */
    {
        const P = await load('powershell.wasm');
        const mP = u8(P);
        const ps = new TextEncoder().encode('Write-Output x');
        mP.set(ps, BASE + 1024);
        const nwPs = P.powershell_move(1024, ps.length, 0, -1);
        ok('§W10 powershell_move -1', nwPs === ps.length && P.powershell_pendente() === ps.length);
        ok('§W10 powershell_corre export', typeof P.powershell_corre === 'function');
        const gotPs = P.powershell_corre();
        ok('§W10 powershell_corre semântica', gotPs > 0);
        const nrPs = P.powershell_move(8192, 32, 8192, +1);
        const gotP = dec.decode(mP.slice(BASE + 8192, BASE + 8192 + nrPs));
        ok('§W10 powershell_move +1', gotP.indexOf('x') >= 0);

        const N = await load('node.wasm');
        const mN = u8(N);
        ok('§W10 node exports', typeof N.node_move === 'function' &&
            typeof N.node_escreve === 'function' &&
            typeof N.node_le === 'function' && typeof N.node_pronto === 'function');
        const js = new TextEncoder().encode('console.log(1)');
        mN.set(js, BASE + 1024);
        const nwN = N.node_move(1024, js.length, 0, -1);
        ok('§W10 node_move -1 aceita script', nwN === js.length);
        ok('§W10 node_pendente', N.node_pendente() === js.length);
        ok('§W10 node_pronto inicia vazio', N.node_pronto() === 0);
        ok('§W10 node_corre export', typeof N.node_corre === 'function');
        const got = N.node_corre();
        ok('§W10 node_corre semântica', got > 0);
        const nrN = N.node_move(8192, 32, 8192, +1);
        const gotN = dec.decode(mN.slice(BASE + 8192, BASE + 8192 + nrN));
        ok('§W10 node_move +1 devolve stdout', gotN.indexOf('1') >= 0);
    }

    /* §W7 Controlo em wasm (8 eixos) */
    {
        const ctl = await load('control.wasm');
        const a = i32(ctl);
        a[0] = 1; // exige_fecho
        for (let i = 0; i < 8; i++) a[1 + i] = 5;
        a[1 + 1] = 0; // theta_R = 0
        a[9] = 0; // R=0
        for (let i = 0; i < 8; i++) a[10 + i] = 0; // D=0
        ok('§W7 RETAIN quando R=0 e D=0', ctl.control_decide() === 0 && a[18] === 0);
        a[9] = 3; // R≠0
        ok('§W7 RETRACT quando R≠0', ctl.control_decide() === 2);
        a[9] = 0; a[10] = 99; // L1 grande
        ok('§W7 MOVE quando D>Θ', ctl.control_decide() === 1);
        a[20] = 2; a[21] = 10; a[22] = 20;
        a[40] = 2; a[41] = 12; a[42] = 18;
        a[60] = 0; a[61] = 0; a[62] = 0; a[63] = 0;
        ctl.control_dist_arena();
        ok('§W7 dist L1=4 caixa=2 forma=2 teclado=0',
            a[10] === 4 && a[14] === 2 && a[15] === 0 && a[17] === 2);
    }

    /* §W11 — cada língua do manifesto: export absorcao.move; MOVE ±1 onde a forma é bytes */
    {
        let nMove = 0;
        for (const L of MAN.linguagens) {
            const ex = await load(L.wasm);
            const fn = L.absorcao && L.absorcao.move;
            const okFn = fn && typeof ex[fn] === 'function';
            if (okFn) nMove++;
            else console.log('  falta export', L.nome, fn);
        }
        ok(`§W11 absorcao.move nas ${MAN.linguagens.length} linguas`, nMove === MAN.linguagens.length);

        const P = await load('emitir.wasm');
        const mP = u8(P);
        const ptx = enc.encode('mov.u32 r1, r2;');
        mP.set(ptx, BASE + 100);
        const n1 = P.emitir(100, ptx.length, 2000);
        const n2 = P.emitir_volta(2000, n1, 4000);
        const ptxBack = dec.decode(mP.slice(BASE + 4000, BASE + 4000 + n2));
        ok('§W11 ptx emitir†=emitir_volta', ptxBack === 'mov.u32 r1, r2;');

        const Lv = await load('baixar.wasm');
        const mL = u8(Lv);
        const ir = enc.encode('define i32 @f()');
        mL.set(ir, BASE + 100);
        const ln = Lv.baixar(100, ir.length, 2000);
        const irBack = dec.decode(mL.slice(BASE + 2000, BASE + 2000 + ln));
        ok('§W11 llvm baixar copia', irBack === 'define i32 @f()');

        const G = await load('filtrar.wasm');
        const aG = i32(G);
        aG[0] = 1; aG[1] = 1;
        G.filtrar(1);
        ok('§W11 glsl filtrar dep', aG[2] === 1);
        aG[0] = 1; aG[1] = 0;
        G.filtrar(1);
        ok('§W11 glsl filtrar recusa', aG[2] === 0);

        const W = await load('empilhar.wasm');
        const sp = W.empilhar_push(7);
        ok('§W11 wasm empilhar push/pop', sp === 1 && W.empilhar_pop() === 7);

        const H = await load('html_compor.wasm');
        const mH = u8(H);
        const html = enc.encode('<p>x</p>');
        mH.set(html, BASE + 100);
        const hn = H.html_move(100, html.length, 2000, -1);
        const hn2 = H.html_move(2000, hn, 4000, +1);
        ok('§W11 html_move ±1', dec.decode(mH.slice(BASE + 4000, BASE + 4000 + hn2)) === '<p>x</p>');
    }

    console.log(`#TOTAL ${feitas} ${falhas}`);
    process.exit(falhas ? 1 : 0);
})().catch(e => {
    console.error(e);
    console.log('#TOTAL', feitas, falhas + 1);
    process.exit(1);
});
