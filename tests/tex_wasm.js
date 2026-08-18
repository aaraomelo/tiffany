/* tex_wasm.js — O CORPO TRADUTOR SOBE INTEIRO: tex.c -> wasm, e o disco cresce contado.
 *
 * O Aarão: «volta à tradução tex ↔ isa webassembly ↔ pdf via navegador» · «não se usa
 * memória aqui» · «vc ta calculando o infinito de novo» (a teoria da medida: contar, não
 * reservar nem aproximar).
 *
 * O `tests/tex.c` — o compositor LaTeX→PDF, ~3900 linhas — sobe pelo `tools/traduz.c` com a
 * `tools/libc.c` e a `lib/spline.h`, e o módulo resultante É VÁLIDO: as 157 funções validam
 * no motor. Não há emscripten, nem clang: é a régua deste repositório a traduzir-se a si
 * própria, C como backend e a ISA/wasm como a roupa que corre no navegador.
 *
 * E a MEDIDA está certa: não há `char MONTE[12M]` (reservar o infinito) nem `realloc` que
 * dobra (aproximar). O disco do módulo começa no que as declarações somam e ESTENDE-SE por
 * `memory.grow` — contado, sob demanda, o mmap do disco.h. Um disco que nunca se escreve não
 * pesa; o que se escreve, cresce.
 *
 *   §T1  o tex.c sobe e o módulo é VÁLIDO — as 157 funções instanciam
 *   §T2  a porta da composição está lá: poe_ficheiro/fopen/fread/fwrite e o DISCO
 *   §T3  o disco CRESCE CONTADO por memory.grow — pede-se 2 MB, sobem ~32 páginas, exacto
 *   §T3b banco MOVE(±1) mesmo end; +1/0 no PDF ainda não nascido
 *   §T3c rascunho: −1 emite, +1 absorve, volta_compila zera
 *   §T4  e o que se escreve no disco lê-se de volta — a Lei 1 no ficheiro, resíduo 0
 *   §T5  Alonzo (a composição): giros, boxed, smallmatrix — /SementeEstrela viaja
 *   §T6  Caelum (o esqueleto): /AssinaturaOito, 256 componentes, dois caminhos batem
 *   §T7  volta_compila: computacional×2 id (1 bit)
 */
'use strict';
const fs = require('fs');
const { execFileSync } = require('child_process');
const path = require('path');

const RAIZ = path.resolve(__dirname, '..');
const TRADUZ = path.join(RAIZ, 'tools', 'bin', 'traduz');
const TMP = process.env.TMPDIR || '/tmp';

let falhas = 0, feitas = 0;
function ok(q, cond) {
    feitas++; if (!cond) falhas++;
    console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`);
    console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`);
}

console.log('=== O CORPO TRADUTOR SOBE INTEIRO: tex.c -> wasm ===\n');

/* a régua constrói-se da fonte, sempre. O fonte é a UNIÃO — libc + spline + tex — com um
 * prelúdio de constantes que os #include dariam (SEEK_*, NULL, stderr como sink). */
const PRELUDIO = '#define SEEK_SET 0\n#define SEEK_CUR 1\n#define SEEK_END 2\n' +
                 '#define NULL 0\n#define EOF (-1)\n#define stderr 0\n#define stdout 1\n#define stdin 3\n' +
                 '#define TEX_COM_LIBC_WASM 1\n';
const w = path.join(TMP, 'tex_wasm.wasm');
const unido = path.join(TMP, 'tex_wasm_unido.c');
try {
    fs.mkdirSync(path.dirname(TRADUZ), { recursive: true });
    execFileSync('cc', ['-O2', '-std=c99', '-w', path.join(RAIZ, 'tools', 'traduz.c'), '-o', TRADUZ]);
    const semInc = (f) => fs.readFileSync(f, 'utf8').split('\n').filter(l => !/^\s*#\s*include/.test(l)).join('\n');
    /* o corte separou o NÚCLEO (tex_core.c) do WRAPPER (tex.c, que o #include). O #include é
     * retirado pelo semInc, logo unem-se os dois explícitos: o núcleo primeiro, e o le_num.h
     * (o strtod/hex sem libc) que ambos usam. É a mesma união do monólito, agora nomeada. */
    fs.writeFileSync(unido, PRELUDIO +
        semInc(path.join(RAIZ, 'tools', 'libc.c')) + '\n' +
        semInc(path.join(RAIZ, 'lib', 'le_num.h')) + '\n' +
        semInc(path.join(RAIZ, 'lib', 'spline.h')) + '\n' +
        semInc(path.join(RAIZ, 'tests', 'tex_core.c')) + '\n' +
        semInc(path.join(RAIZ, 'tests', 'tex.c')));
    /* o traduz ignora linhas `#` (não é o cpp). Sem expandir, MAXLIN/SEEK_END
     * ficam nomes a zero — a mesma costura de tools/sobe_tex_wasm.sh. */
    const unidoPp = path.join(TMP, 'tex_wasm_unido_pp.c');
    execFileSync('cc', ['-E', '-P', unido, '-o', unidoPp]);
    execFileSync(TRADUZ, [unidoPp, '-o', w]);
} catch (e) {
    console.log('  o tex.c nao subiu — e sem ele nao ha nada a medir.');
    console.log(String(e.stderr || e.message).slice(0, 400));
    console.log('#TOTAL 0 1'); process.exit(1);
}

const bytes = fs.readFileSync(w);
const { instanciaTex } = require('./tex_env.js');
const M = new WebAssembly.Module(bytes);
const EXP = WebAssembly.Module.exports(M);

/* ─── §T1 o modulo e VALIDO ───────────────────────────────────────────────────────── */
/* conta as funcoes TOTAIS pela seccao de codigo (as static nao se exportam), nao so as da porta */
const nfun = (() => {
    let p = 8; const b = bytes;
    const leb = () => { let v = 0, s = 0, c; do { c = b[p++]; v |= (c & 0x7f) << s; s += 7; } while (c & 0x80); return v; };
    while (p < b.length) { const id = b[p++], t = leb(); const e2 = p + t; if (id === 10) return leb(); p = e2; }
    return 0;
})();
console.log(`   ${nfun} funcoes (total), ${bytes.length} bytes de wasm — o modulo compilou`);
ok('§T1 o tex.c sobe e o modulo e VALIDO — sem emscripten, a regua traduz-se a si propria',
   nfun > 100 && bytes.length > 50000);

/* ─── §T2 a porta da composicao ───────────────────────────────────────────────────── */
const tem = (n, k) => EXP.some(x => x.name === n && x.kind === (k || 'function'));
const porta = ['poe_ficheiro', 'fopen', 'fread', 'fwrite', 'ftell', 'fseek',
               'inicia_wasm', 'compila_ficheiro', 'limpa_saida', 'end_saida', 'tam_saida',
               'MOVE', 'end_fatia', 'vfs_reserva', 'marca_vfs', 'volta_compila'].every(n => tem(n));
console.log(`   porta: DISCO(${tem('DISCO', 'memory')}) MOVE fatias poe inicia compila ...`);
ok('§T2 a porta da composicao esta la: MOVE no DISCO, fatias e ficheiros por slots',
   tem('DISCO', 'memory') && porta);

/* o motor pode nao instanciar sob o ulimit -v da bateria (vale para TODOS os modulos com
 * disco): entao §T3/§T4 dizem o que se pode, e afirmam so o verificado. */
let E = null;
try { E = instanciaTex(M).exports; }
catch (e) {
    if (!/Out of memory|Cannot allocate/i.test(e.message)) throw e;
    console.log('\n   o motor nao instancia modulos com disco sob este limite de espaco virtual');
    console.log('   — e o caso de TODOS os modulos com disco; a validade foi medida acima.');
    ok('§T3 o disco declara min (as declaracoes) e max (o tecto do memory.grow) — verificado no binario',
       (() => {  /* le a seccao 5: min < max prova o disco crescivel, nao reservado */
           let p = 8; const b = bytes;
           const leb = () => { let v = 0, s = 0, c; do { c = b[p++]; v |= (c & 0x7f) << s; s += 7; } while (c & 0x80); return v; };
           while (p < b.length) { const id = b[p++], t = leb(); const e2 = p + t;
               if (id === 5) { leb(); const fl = b[p++]; const mn = leb(); const mx = fl ? leb() : mn; return fl === 1 && mx > mn; }
               p = e2; }
           return false;
       })());
    console.log(`#TOTAL ${feitas} ${falhas}`);
    process.exit(falhas ? 1 : 0);
}

/* ─── §T3 o disco cresce CONTADO ──────────────────────────────────────────────────── */
{
    const pag0 = E.DISCO.buffer.byteLength / 65536;
    const p = Number(E.malloc(1n << 21n));      /* 2 MB */
    const pag1 = E.DISCO.buffer.byteLength / 65536;
    const cresceu = pag1 - pag0;
    console.log(`   pediu 2 MB: disco ${pag0} -> ${pag1} paginas (cresceu ${cresceu}, ~32 esperadas)`);
    ok('§T3 o disco CRESCE CONTADO por memory.grow — nao reservado, nao 12 MB fixos',
       p > 0 && cresceu >= 32 && cresceu <= 34);
}

/* ─── §T3b MOVE ±1 no banco: mesmo endereço, host escreve e lê (painel) ─────────── */
{
    E.inicia_wasm();
    const slot = 1;   /* TAM[1] = 64 KB — banco (classe); rascunho 3–15 ainda não nasceu */
    const a = Number(E.MOVE(slot, -1));   /* emite: garante */
    const b = Number(E.MOVE(slot, 1));    /* absorve: mesmo end no banco */
    const tam = Number(E.tam_fatia(slot));
    const oct = new Uint8Array(E.DISCO.buffer);
    for (let i = 0; i < 64; i++) oct[a + i] = (i * 7 + 3) & 255;
    let mau = 0;
    for (let i = 0; i < 64; i++) if (oct[b + i] !== ((i * 7 + 3) & 255)) mau++;
    const pdfAbs = Number(E.MOVE(14, 1));           /* +1 não nasce */
    const pdfAtr = Number(E.MOVE(14, 0));           /* 0 atravessa: idem */
    const end14 = Number(E.end_fatia(14));
    const cap14 = Number(E.tam_fatia(14));
    const inicia = E.DISCO.buffer.byteLength;
    console.log(`   §T3b MOVE(1,±1) end=${a}==${b} tam=${tam} mau=${mau}; slot14+1=${pdfAbs} atr=${pdfAtr} cap=${cap14} inicia=${(inicia/1048576).toFixed(2)} MiB`);
    ok('§T3b banco: MOVE(±1) mesmo endereço; +1/0 no PDF ainda não nascido — residuo 0',
       a > 0 && a === b && mau === 0 && tam === (1 << 16)
       && pdfAbs === 0 && pdfAtr === 0 && end14 === 0
       && cap14 === (1 << 27) && inicia < 20 * 1048576);
}

/* ─── §T3c −1 emite rascunho; +1 só lê; volta_compila apaga ─────────────────────── */
{
    const emi = Number(E.MOVE(5, -1));
    const abs = Number(E.MOVE(5, 1));
    const atr = Number(E.MOVE(5, 0));
    const oct = new Uint8Array(E.DISCO.buffer);
    for (let i = 0; i < 32; i++) oct[emi + i] = (i + 9) & 255;
    let mau = 0;
    for (let i = 0; i < 32; i++) if (oct[abs + i] !== ((i + 9) & 255)) mau++;
    if (typeof E.volta_compila === 'function') E.volta_compila();
    const apos = Number(E.MOVE(5, 1));
    console.log(`   §T3c emite=${emi} absorve=${abs} atr=${atr} mau=${mau} após volta +1=${apos}`);
    ok('§T3c rascunho: −1 nasce, +1 lê o mesmo, volta_compila zera (+1→0)',
       emi > 0 && emi === abs && emi === atr && mau === 0 && apos === 0);
}

/* ─── §T4 o que se escreve le-se de volta ─────────────────────────────────────────── */
{
    const r = Number(E.malloc(8192n));          /* aloca ANTES; a vista vem depois de crescer */
    const oct = new Uint8Array(E.DISCO.buffer); /* re-vinculada — o malloc pode ter crescido */
    const enc = s => Buffer.from(s, 'latin1');
    const poe = (a, s) => { const b = enc(s); for (let i = 0; i < b.length; i++) oct[a + i] = b[i]; oct[a + b.length] = 0; return a; };
    const conteudo = 'reino dourado\ne o corpo\n';
    E.poe_ficheiro(poe(r, 'lib/x.txt'), poe(r + 256, conteudo), enc(conteudo).length);
    /* abre COM `../` a frente, como o compositor faz: quem abre resolve pelo sufixo */
    const h = E.fopen(poe(r + 1024, '../lib/x.txt'), poe(r + 1200, 'r'));
    const dest = r + 2048;
    const n = E.fread(dest, 1, 4096, h);
    let volta = ''; for (let i = 0; i < n; i++) volta += String.fromCharCode(oct[dest + i]);
    console.log(`   §T4 poe_ficheiro/fopen/fread: ${n} bytes, iguais? ${volta === conteudo}`);
    ok('§T4 o ficheiro e um SLOT: o que o host poe, o compositor le de volta — residuo 0',
       n === conteudo.length && volta === conteudo);
}

/* ─── §T5 Alonzo: a composição; §T6 Caelum: o selo ───────────────────────────────────
 * O que Alonzo compõe, Caelum assina (catalogo.tex cat:alonzo / cat:caelum; tex.c §X13/§X15). */
{
    const man = JSON.parse(fs.readFileSync(path.join(RAIZ, 'app/src/corpo.json'), 'utf8'));
    const num = x => typeof x === 'bigint' ? Number(x) : x;
    const mem = () => new Uint8Array(E.DISCO.buffer);
    function reserva(n) {
        const p = num(E.vfs_reserva(n));
        if (!p) throw new Error('vfs_reserva');
        return p;
    }
    function poeStr(s) {
        const nb = Buffer.from(s, 'latin1');
        const p = reserva(nb.length + 1);
        mem().set(nb, p); mem()[p + nb.length] = 0;
        return p;
    }
    function poeFich(nome, bytes) {
        const pN = poeStr(nome);
        const pD = reserva(Math.max(bytes.length, 0) + 1);
        if (bytes.length) mem().set(bytes instanceof Buffer ? bytes : Buffer.from(bytes), pD);
        mem()[pD + bytes.length] = 0;
        if (!E.poe_ficheiro(pN, pD, bytes.length)) throw new Error('poe_ficheiro recusou ' + nome);
    }
    function compoe(nome) {
        E.limpa_saida();
        const rc = num(E.compila_ficheiro(poeStr(nome), poeStr('saida.pdf')));
        const tam = num(E.tam_saida());
        const end = num(E.MOVE(14, 1));
        const pdf = Buffer.from(mem().slice(end, end + tam));
        return { rc, tam, pdf };
    }
    function acha(pdf, marca) {
        const m = Buffer.from(marca, 'latin1');
        return pdf.indexOf(m);
    }
    /* o selo de Caelum: streams das Forms → N=2^8, Z_65537, raiz 3^256 — tex.c §X15 */
    function espectro(pdf) {
        const A = new Array(256).fill(0);
        const marca = Buffer.from('/Type/XObject/Subtype/Form', 'latin1');
        const ini = Buffer.from('stream\n', 'latin1');
        const fim = Buffer.from('endstream', 'latin1');
        let q = 0;
        while (q < pdf.length) {
            const p = pdf.indexOf(marca, q); if (p < 0) break;
            const a = pdf.indexOf(ini, p); if (a < 0) break;
            const b = pdf.indexOf(fim, a + 7); if (b < 0) break;
            let k = 0;
            for (let i = a + 7; i < b; i++) { A[k & 255] = (A[k & 255] + pdf[i]) % 65537; k++; }
            q = b + 9;
        }
        let raiz = 1, b3 = 3, e3 = 256;
        while (e3 > 0) { if (e3 & 1) raiz = raiz * b3 % 65537; b3 = b3 * b3 % 65537; e3 >>= 1; }
        const S = new Array(256);
        for (let j = 0; j < 256; j++) {
            let acc = 0, w = 1, passo = 1, e4 = j, b4 = raiz;
            while (e4 > 0) { if (e4 & 1) passo = passo * b4 % 65537; b4 = b4 * b4 % 65537; e4 >>= 1; }
            for (let t = 0; t < 256; t++) { acc = (acc + A[t] * w) % 65537; w = w * passo % 65537; }
            S[j] = acc;
        }
        return S;
    }
    function leSelo(pdf) {
        const p = acha(pdf, '/Type/AssinaturaOito');
        if (p < 0) return null;
        const a = pdf.indexOf(0x5b, p); /* '[' */
        if (a < 0) return null;
        const SW = [];
        let w = a + 1;
        while (w < pdf.length && pdf[w] !== 0x5d && SW.length < 256) {
            while (w < pdf.length && pdf[w] === 0x20) w++;
            if (pdf[w] === 0x5d) break;
            let u = 0;
            while (w < pdf.length && pdf[w] >= 0x30 && pdf[w] <= 0x39) { u = u * 10 + (pdf[w] - 0x30); w++; }
            SW.push(u);
        }
        return SW;
    }
    function leSemente(pdf) {
        const p = acha(pdf, '/Type/SementeEstrela');
        if (p < 0) return null;
        const v = [];
        for (let w = p + 20; w < pdf.length && v.length < 4; w++) {
            if (pdf[w] >= 0x30 && pdf[w] <= 0x39) {
                let u = 0;
                while (w < pdf.length && pdf[w] >= 0x30 && pdf[w] <= 0x39) { u = u * 10 + (pdf[w] - 0x30); w++; }
                v.push(u);
            }
            if (pdf[w] === 0x3e) break; /* '>' */
        }
        return v;
    }

    const ALONZO =
        '\\documentclass{article}\n\\begin{document}\n' +
        '\\[ \\left(\\frac{a^{b^{c}}}{x}\\right) \\qquad' +
        ' \\boxed{\\ \\sigma\\,\\sigma = -1\\ } \\qquad' +
        ' \\bigl(\\begin{smallmatrix}0&b\\\\-b&0\\end{smallmatrix}\\bigr) \\]\n' +
        '\\end{document}\n';
    const CAELUM =
        '\\documentclass{article}\n\\begin{document}\n' +
        'O esqueleto assina: $x^{2}$ e $\\frac{a}{b}$ e texto.\n' +
        '\\end{document}\n';

    let trap = '';
    let aRc = -1, aTam = 0, aEof = false, aSem = null, aForms = 0;
    let cRc = -1, cTam = 0, cEof = false, cSelo = null, cBate = false;
    try {
        E.inicia_wasm();
        for (const f of man.ficheiros) poeFich(f, fs.readFileSync(path.join(RAIZ, f)));
        poeFich('alonzo.tex', Buffer.from(ALONZO, 'latin1'));
        poeFich('caelum.tex', Buffer.from(CAELUM, 'latin1'));
        if (typeof E.marca_vfs === 'function') E.marca_vfs();

        const A = compoe('alonzo.tex');
        aRc = A.rc; aTam = A.tam;
        aEof = A.pdf.includes(Buffer.from('%%EOF'));
        aSem = leSemente(A.pdf);
        aForms = (A.pdf.toString('latin1').match(/\/Subtype\/Form/g) || []).length;

        /* 1 bit zera FICH (banco = LS/Map); o selo de Caelum precisa do corpo de novo. */
        if (typeof E.volta_compila === 'function') E.volta_compila();
        for (const f of man.ficheiros) poeFich(f, fs.readFileSync(path.join(RAIZ, f)));
        poeFich('caelum.tex', Buffer.from(CAELUM, 'latin1'));
        if (typeof E.marca_vfs === 'function') E.marca_vfs();
        const C = compoe('caelum.tex');
        cRc = C.rc; cTam = C.tam;
        cEof = C.pdf.includes(Buffer.from('%%EOF'));
        cSelo = leSelo(C.pdf);
        if (cSelo && cSelo.length === 256) {
            const S = espectro(C.pdf);
            cBate = S.every((v, i) => v === cSelo[i]);
        }
    } catch (e) { trap = String(e && e.message || e).slice(0, 200); }

    console.log(`   §T5 Alonzo rc=${aRc} bytes=${aTam} Forms=${aForms} semente=${aSem && aSem.join(',')} eof=${aEof}${trap ? ' trap=' + trap : ''}`);
    ok('§T5 Alonzo (a composição): giros, boxed, smallmatrix — /SementeEstrela 3,17,20,4 e %%EOF',
       !trap && aRc === 0 && aEof && aForms > 0 && aSem && aSem[0] === 3 && aSem[1] === 17 && aSem[2] === 20 && aSem[3] === 4);

    console.log(`   §T6 Caelum rc=${cRc} bytes=${cTam} selo=${cSelo && cSelo.length} bate=${cBate} eof=${cEof}`);
    ok('§T6 Caelum (o esqueleto): /AssinaturaOito 256, dois caminhos batem — o que Alonzo compõe, Caelum assina',
       !trap && cRc === 0 && cEof && cSelo && cSelo.length === 256 && cBate);

    /* §T7 1 bit + miss: instância fresca — computacional×2, Map → fopen, FICH zera. */
    let d1 = { rc: -1, tam: 0, forms: 0 }, d2 = { rc: -1, tam: 0, forms: 0 };
    try {
        const cache7 = new Map(man.ficheiros.map((f) => [f, fs.readFileSync(path.join(RAIZ, f))]));
        const poeSet7 = new Set();
        let E7 = null;
        const mem7 = () => new Uint8Array(E7.DISCO.buffer);
        const res7 = (n) => { const p = num(E7.vfs_reserva(n)); if (!p) throw new Error('vfs'); return p; };
        const str7 = (s) => { const nb = Buffer.from(s, 'latin1'); const p = res7(nb.length + 1); mem7().set(nb, p); mem7()[p + nb.length] = 0; return p; };
        const poe7 = (nome, bytes) => {
            const pN = str7(nome); const pD = res7(Math.max(bytes.length, 0) + 1);
            if (bytes.length) mem7().set(bytes instanceof Buffer ? bytes : Buffer.from(bytes), pD);
            mem7()[pD + bytes.length] = 0;
            if (!E7.poe_ficheiro(pN, pD, bytes.length)) throw new Error('poe ' + nome);
        };
        const { hitCorpo } = require('./tex_env.js');
        const hit7 = (nome) => hitCorpo(cache7, nome);
        E7 = instanciaTex(M, (ptr) => {
            const v = mem7(); let s = '';
            for (let i = ptr; i < v.length && v[i]; i++) s += String.fromCharCode(v[i]);
            const h = hit7(s); if (!h) return 0;
            if (poeSet7.has(h.nome)) return 1;
            poe7(h.nome, h.u8); poeSet7.add(h.nome); return 1;
        }).exports;
        E7.inicia_wasm();
        if (typeof E7.marca_vfs === 'function') E7.marca_vfs();
        const comp7 = (nome) => {
            E7.limpa_saida();
            const rc = num(E7.compila_ficheiro(str7(nome), str7('saida.pdf')));
            const tam = num(E7.tam_saida());
            const end = num(E7.MOVE(14, 1));
            const pdf = Buffer.from(mem7().slice(end, end + tam));
            return { rc, tam, pdf };
        };
        if (typeof E7.volta_compila === 'function') E7.volta_compila();
        poeSet7.clear();
        d1 = comp7('papers/corpo_computacional.tex');
        if (typeof E7.volta_compila === 'function') E7.volta_compila();
        poeSet7.clear();
        d2 = comp7('papers/corpo_computacional.tex');
    } catch (e) { trap = String(e && e.message || e).slice(0, 200); }
    const f1 = d1.pdf ? (d1.pdf.toString('latin1').match(/\/Subtype\/Form/g) || []).length : 0;
    const f2 = d2.pdf ? (d2.pdf.toString('latin1').match(/\/Subtype\/Form/g) || []).length : 0;
    console.log(`   §T7 computacional×2 rc=${d1.rc}/${d2.rc} tam=${d1.tam}/${d2.tam} Forms=${f1}/${f2}`);
    ok('§T7 volta_compila: a segunda composição é a mesma (1 bit, sem recorrência) — resíduo 0',
       !trap && d1.rc === 0 && d2.rc === 0 && d1.tam === d2.tam && f1 === f2 && f1 > 50 && d1.tam > 1e5);
}

console.log('\n==========================================================================');
if (!falhas) {
    console.log('  O compositor LaTeX->PDF sobe INTEIRO para wasm e o modulo e valido.');
    console.log('  Sem emscripten. C e backend; a ISA/wasm e a roupa do navegador.');
    console.log('');
    console.log('  A composição é o corpo de Alonzo; o selo é o de Caelum.');
    console.log('  Disco = fatias + MOVE(slot, ±1), isomorfo ao mmap e à ISA em assembly.');
    console.log('  O PDF lê-se do slot 14; o que se escreve lê-se de volta.');
} else console.log(`  FALHOU: ${falhas}`);
console.log(`#TOTAL ${feitas} ${falhas}`);
process.exit(falhas ? 1 : 0);
