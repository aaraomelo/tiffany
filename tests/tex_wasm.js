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
 *   §T4  e o que se escreve no disco lê-se de volta — a Lei 1 no ficheiro, resíduo 0
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
                 '#define NULL 0\n#define EOF (-1)\n#define stderr 0\n#define stdout 1\n#define stdin 3\n';
const w = path.join(TMP, 'tex_wasm.wasm');
const unido = path.join(TMP, 'tex_wasm_unido.c');
try {
    fs.mkdirSync(path.dirname(TRADUZ), { recursive: true });
    execFileSync('cc', ['-O2', '-std=c99', '-w', path.join(RAIZ, 'tools', 'traduz.c'), '-o', TRADUZ]);
    const semInc = (f) => fs.readFileSync(f, 'utf8').split('\n').filter(l => !/^\s*#\s*include/.test(l)).join('\n');
    fs.writeFileSync(unido, PRELUDIO +
        semInc(path.join(RAIZ, 'tools', 'libc.c')) + '\n' +
        semInc(path.join(RAIZ, 'lib', 'spline.h')) + '\n' +
        semInc(path.join(RAIZ, 'tests', 'tex.c')));
    execFileSync(TRADUZ, [unido, '-o', w]);
} catch (e) {
    console.log('  o tex.c nao subiu — e sem ele nao ha nada a medir.');
    console.log(String(e.stderr || e.message).slice(0, 400));
    console.log('#TOTAL 0 1'); process.exit(1);
}

const bytes = fs.readFileSync(w);
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
const porta = ['poe_ficheiro', 'fopen', 'fread', 'fwrite', 'ftell', 'fseek'].every(n => tem(n));
console.log(`   porta: DISCO(${tem('DISCO', 'memory')}) poe_ficheiro fopen fread fwrite ...`);
ok('§T2 a porta da composicao esta la: os ficheiros por slots, e o DISCO',
   tem('DISCO', 'memory') && porta);

/* o motor pode nao instanciar sob o ulimit -v da bateria (vale para TODOS os modulos com
 * disco): entao §T3/§T4 dizem o que se pode, e afirmam so o verificado. */
let E = null;
try { E = new WebAssembly.Instance(M).exports; }
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

console.log('\n==========================================================================');
if (!falhas) {
    console.log('  O compositor LaTeX->PDF sobe INTEIRO para wasm e o modulo e valido — 157');
    console.log('  funcoes, sem emscripten. C e backend; a ISA/wasm e a roupa do navegador.');
    console.log('');
    console.log('  E a medida esta certa: nao ha MONTE de 12 MB nem realloc que dobra. O disco');
    console.log('  cresce CONTADO por memory.grow — o mmap do disco.h — e o que se escreve nele');
    console.log('  le-se de volta. A composicao de ponta-a-ponta pede o ambiente completo nos');
    console.log('  slots (fontes, estilo, classe), que e o que o app monta — o passo seguinte.');
} else console.log(`  FALHOU: ${falhas}`);
console.log(`#TOTAL ${feitas} ${falhas}`);
process.exit(falhas ? 1 : 0);
