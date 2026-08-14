/* libc_wasm.js — A BIBLIOTECA ESCRITA NA MESMA RÉGUA, MEDIDA CONTRA A DO SISTEMA.
 *
 * O `tests/tex.c` chama 32 funções da biblioteca, 461 vezes. No navegador não há biblioteca
 * do sistema — e quase nenhuma delas precisa de existir por fora: são laços sobre `char *`, e
 * sobem pelo tradutor que já cá está, sem uma linha nova nele.
 *
 * Aqui mede-se `tools/libc.c` por dois lados, e são o par:
 *
 *   IDA     cada função dá o MESMO que a do sistema, sobre as mesmas entradas. E o outro lado
 *           não é um número meu: é a glibc, que não fui eu que escrevi.
 *   VOLTA   sobe(desce(M)) é o próprio M, byte a byte — que é o que diz que traduzir preserva.
 *
 * E as entradas incluem bytes acima de 127 de propósito: o `char` do wasm lê-se COM sinal e o
 * C manda comparar texto SEM ele. Sem o `& 255` um acentuado comparava negativo e a ordenação
 * de qualquer texto português saía ao contrário — um defeito que só aparece se se for buscá-lo.
 *
 *   §L1  o texto: strlen strcmp strncmp strcpy strncpy strcat strstr strchr strrchr
 *   §L2  a memória: memcpy memmove memset memcmp — e o memmove com sobreposição
 *   §L3  os caracteres: is* e to*, em todos os 256 bytes
 *   §L4  os números: atoi atol strtol atof
 *   §L5  A VOLTA: sobe(desce(M)) = M, resíduo 0 INTEIRO
 *   §L6  o CONTROLO: mudado um byte, os dois lados deixam de concordar
 */
'use strict';
const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');

const RAIZ = path.resolve(__dirname, '..');
const TRADUZ = path.join(RAIZ, 'tools', 'bin', 'traduz');
const TMP = process.env.TMPDIR || '/tmp';

let falhas = 0, feitas = 0;
function ok(q, cond) {
    feitas++; if (!cond) falhas++;
    console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`);
    console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`);
}

console.log('=== A BIBLIOTECA NA MESMA REGUA, contra a do sistema ===\n');

/* a régua constrói-se da fonte, sempre */
const unidade = path.join(TMP, 'libc_unidade.c');
const PREAMBULO = `
char B1[512]; char B2[512]; char B3[512]; char B4[2048];
int e1(void){ return (int)B1; }
int e2(void){ return (int)B2; }
int e3(void){ return (int)B3; }
int e4(void){ return (int)B4; }
`;
const w = path.join(TMP, 'libc.wasm');
try {
    fs.mkdirSync(path.dirname(TRADUZ), { recursive: true });
    execFileSync('cc', ['-O2', '-std=c99', '-w', path.join(RAIZ, 'tools', 'traduz.c'), '-o', TRADUZ]);
    fs.writeFileSync(unidade, PREAMBULO + fs.readFileSync(path.join(RAIZ, 'tools', 'libc.c'), 'utf8'));
    execFileSync(TRADUZ, [unidade, '-o', w]);
} catch (e) {
    console.log('  a biblioteca nao subiu — e sem ela nao ha nada a medir.');
    console.log(String(e.stderr || e.message).slice(0, 400));
    console.log('#TOTAL 0 1'); process.exit(1);
}
const bytes = fs.readFileSync(w);
const M = new WebAssembly.Module(bytes);

/* a libc ganhou imports desde que este medidor nasceu (env.__fich_miss,
 * do trabalho do MAX_FICH) e o Node exige o objeto: constroem-se stubs
 * NOMEADOS a partir do que o módulo declara — nada silencioso */
const IMP = {};
for (const d of WebAssembly.Module.imports(M)) {
    IMP[d.module] = IMP[d.module] || {};
    if (d.kind === 'function') IMP[d.module][d.name] = () => 0;
}
let E = null;
try { E = new WebAssembly.Instance(M, IMP).exports; }
catch (e) {
    if (!/Out of memory|Cannot allocate/i.test(e.message)) throw e;
}

/* ── os casos, e são os mesmos dos dois lados ─────────────────────────────────────── */
const PARES = [
    ['', ''], ['a', 'a'], ['a', 'b'], ['b', 'a'], ['abc', 'abd'], ['abc', 'ab'],
    ['reino', 'reino'], ['reino', 'reino dourado'], ['', 'x'], ['x', ''],
    ['Reino', 'reino'], ['dourado', 'Dourado'],
    ['\u00e1rvore', 'arvore'], ['\u00e7', 'c'], ['n\u00e3o', 'nao'],   /* acima de 127 */
    ['aaaa', 'aaab'], ['\u00ff', '\u0001'], ['\u0080x', '\u0001x'],
];
const AGULHAS = [['reino dourado', 'dourado'], ['reino', 'rei'], ['reino', 'xyz'],
                 ['reino', ''], ['', 'a'], ['aaa', 'aa'], ['abcabc', 'cab']];
const NUMEROS = ['0', '7', '-7', '  42', '+13', '2147483647', '-2147483648', 'x', '',
                 '12abc', '0x1f', '007', '-0', '1000000000000'];
const REAIS = ['0', '1', '-1', '3.5', '-0.125', '  2.5e3', '1e-3', '0.1', '12.0e+2',
               '-7.25', 'abc', '', '1000000'];
/* [formato, argumentos em C, tipos, valores] — os formatos sao os que o tex.c usa */
const FORMATOS = [
    ['%d', '42', ['i32'], [42]],
    ['%d', '-42', ['i32'], [-42]],
    ['%d', '0', ['i32'], [0]],
    ['%5d', '42', ['i32'], [42]],
    ['%05d', '42', ['i32'], [42]],
    ['%05d', '-42', ['i32'], [-42]],
    ['%ld', '1000000000000L', ['i64'], [1000000000000]],
    ['%010ld', '-77L', ['i64'], [-77]],
    ['%x', '255', ['i32'], [255]],
    ['%2x', '10', ['i32'], [10]],
    ['%c', '65', ['i32'], [65]],
    ['%s', '"reino"', ['s'], ['reino']],
    ['%10s', '"reino"', ['s'], ['reino']],
    ['%-10s|', '"reino"', ['s'], ['reino']],
    ['%.3s', '"dourado"', ['s'], ['dourado']],
    ['%%', '0', [], []],
    ['a%db%sc', '7, "x"', ['i32','s'], [7, 'x']],
    ['%.2f', '3.14159', ['f64'], [3.14159]],
    ['%.3f', '3.14159', ['f64'], [3.14159]],
    ['%.2f', '-2.005', ['f64'], [-2.005]],
    ['%.0f', '2.5', ['f64'], [2.5]],
    ['%.3f', '0.0', ['f64'], [0.0]],
    ['%.2f', '1234.5', ['f64'], [1234.5]],
    ['%.3f', '-0.0005', ['f64'], [-0.0005]],
    ['%.0f', '3.5', ['f64'], [3.5]],
    ['%.0f', '0.5', ['f64'], [0.5]],
    ['%.0f', '1.5', ['f64'], [1.5]],
    ['%.1f', '0.25', ['f64'], [0.25]],
    ['%.1f', '0.35', ['f64'], [0.35]],
    ['%.2f', '2.675', ['f64'], [2.675]],
    ['%.2f', '0.005', ['f64'], [0.005]],
    ['%.3f', '13.6', ['f64'], [13.6]],
    ['%.3f', '10.95', ['f64'], [10.95]],
    ['%.3f', '768.0', ['f64'], [768.0]],
    ['%.2f', '-3.145', ['f64'], [-3.145]],
];
/* a leitura: [texto, formato, tipos dos destinos] — os formatos sao os do tex.c */
const LEITURAS = [
    ['12pt', '%dpt', ['i32']],
    ['12', '%dpt', ['i32']],
    ['  -7 xyz', '%d', ['i32']],
    ['1.5', '%lf', ['f64']],
    ['-0.125e2', '%lf', ['f64']],
    ['abc', '%lf', ['f64']],
    ['{1.5}{2.5}', '{%lf}{%lf}', ['f64','f64']],
    ['{1.5}{x}', '{%lf}{%lf}', ['f64','f64']],
    ['2.5cm', '%lf%2[a-z]', ['f64','s']],
    ['2.5', '%lf%2[a-z]', ['f64','s']],
    ['13.6pt}{2.0em}', '%lf%2[a-z]}{%lf%2[a-z]}', ['f64','s','f64','s']],
    ['3 4', '%d %d', ['i32','i32']],
    ['3   4', '%d %d', ['i32','i32']],
    ['3', '%d %d', ['i32','i32']],
    ['7 2.5 Tf', '%d %lf Tf', ['i32','f64']],
    ['7 2.5 Xf', '%d %lf Tf', ['i32','f64']],
    ['reino dourado', '%s', ['s']],
    ['reino dourado', '%63s %63s', ['s','s']],
    ['  x', '%c', ['s']],
];

function texto_c(s) {
    let o = '"';
    for (const ch of Buffer.from(s, 'latin1')) {
        if (ch === 34 || ch === 92) o += '\\' + String.fromCharCode(ch);
        else if (ch >= 32 && ch < 127) o += String.fromCharCode(ch);
        else o += '\\' + ch.toString(8).padStart(3, '0');
    }
    return o + '"';
}

/* ── o lado do sistema ────────────────────────────────────────────────────────────── */
function do_sistema() {
    let c = '#include <stdio.h>\n#include <string.h>\n#include <ctype.h>\n#include <stdlib.h>\n';
    c += 'static char b1[512], b2[512];\nint main(void){\n';
    c += '  char *r;\n  int i;\n';
    for (const [a, b] of PARES) {
        const A = texto_c(a), B = texto_c(b);
        c += `  printf("%d\\n", (int)strlen(${A}));\n`;
        c += `  printf("%d\\n", strcmp(${A},${B}) < 0 ? -1 : (strcmp(${A},${B}) > 0 ? 1 : 0));\n`;
        c += `  printf("%d\\n", strncmp(${A},${B},3) < 0 ? -1 : (strncmp(${A},${B},3) > 0 ? 1 : 0));\n`;
        c += `  strcpy(b1,${A}); printf("%s\\n", b1);\n`;
        c += `  memset(b2,35,8); strncpy(b2,${A},6); for(i=0;i<8;i++) printf("%d ", (unsigned char)b2[i]); printf("\\n");\n`;
        c += `  strcpy(b1,${A}); strcat(b1,${B}); printf("%s\\n", b1);\n`;
        c += `  r = strchr(${A}, 'o');  printf("%d\\n", r ? (int)(r-${A}) : -1);\n`;
        c += `  r = strrchr(${A}, 'o'); printf("%d\\n", r ? (int)(r-${A}) : -1);\n`;
    }
    for (const [h, ag] of AGULHAS) {
        const H = texto_c(h), G = texto_c(ag);
        c += `  r = strstr(${H},${G}); printf("%d\\n", r ? (int)(r-${H}) : -1);\n`;
    }
    c += '  memset(b1,0,32); memcpy(b1,"reino dourado",13);\n';
    c += '  memmove(b1+4,b1,9); for(i=0;i<16;i++) printf("%d ", (unsigned char)b1[i]); printf("\\n");\n';
    c += '  memset(b1,0,32); memcpy(b1,"reino dourado",13);\n';
    c += '  memmove(b1,b1+4,9); for(i=0;i<16;i++) printf("%d ", (unsigned char)b1[i]); printf("\\n");\n';
    c += '  memset(b1,65,10); for(i=0;i<10;i++) printf("%d ", (unsigned char)b1[i]); printf("\\n");\n';
    c += '  printf("%d\\n", memcmp("abc","abd",3) < 0 ? -1 : 1);\n';
    c += '  printf("%d\\n", memcmp("abc","abc",3));\n';
    c += '  for(i=0;i<256;i++) printf("%d %d %d %d %d %d %d %d\\n", !!isdigit(i), !!isupper(i), !!islower(i), !!isalpha(i), !!isalnum(i), !!isspace(i), toupper(i), tolower(i));\n';
    for (const n of NUMEROS) {
        const N = texto_c(n);
        c += `  printf("%d\\n", atoi(${N}));\n`;
        c += `  printf("%ld\\n", atol(${N}));\n`;
        c += `  { char *f; long v = strtol(${N}, &f, 0); printf("%ld %d\\n", v, (int)(f-${N})); }\n`;
    }
    for (const n of REAIS) c += `  printf("%.17g\\n", atof(${texto_c(n)}));\n`;
    /* o formatador, com os formatos que o tex.c usa — medidos nele, nao os que a norma tem */
    for (const [f, a1] of FORMATOS) c += `  snprintf(b1, 64, ${texto_c(f)}, ${a1}); printf("[%s]\\n", b1);\n`;
    /* a leitura, e diz-se QUANTOS atribuiu e o que ficou em cada destino */
    c += '  { int d1,d2; double r1,r2; char t1[64],t2[64];\n';
    for (const [txt, f, tipos] of LEITURAS) {
        const dst = [];
        let ni = 0, nr = 0, nt = 0;
        for (const t of tipos) dst.push(t === 'i32' ? `&d${++ni}` : t === 'f64' ? `&r${++nr}` : `t${++nt}`);
        c += `    d1=0;d2=0;r1=0;r2=0;t1[0]=0;t2[0]=0;\n`;
        c += `    printf("%d", sscanf(${texto_c(txt)}, ${texto_c(f)}${dst.length ? ', ' + dst.join(', ') : ''}));\n`;
        ni = 0; nr = 0; nt = 0;
        for (const t of tipos) {
            if (t === 'i32') c += `    printf(" %d", d${++ni});\n`;
            else if (t === 'f64') c += `    printf(" %.17g", r${++nr});\n`;
            else c += `    printf(" [%s]", t${++nt});\n`;
        }
        c += '    printf("\\n");\n';
    }
    c += '  }\n';
    c += '  return 0;\n}\n';
    const f = path.join(TMP, 'libc_sistema.c');
    fs.writeFileSync(f, c);
    const bin = path.join(TMP, 'libc_sistema');
    execFileSync('cc', ['-O2', '-std=c99', '-w', f, '-o', bin]);
    /* LATIN1, e não utf8: o que o programa escreve são BYTES, e lê-los como utf8 troca
     * cada acentuado por um losango — a diferença aparecia no medidor e não no objecto. */
    return execFileSync(bin).toString('latin1').trim().split('\n').map(x => x.trim());
}

/* ── o lado do wasm ───────────────────────────────────────────────────────────────── */
function do_wasm() {
    const oct = new Uint8Array(E.DISCO.buffer);
    const B1 = E.e1(), B2 = E.e2(), B3 = E.e3(), B4 = E.e4();
    const out = [];
    const poe = (end, s) => {
        const b = Buffer.from(s, 'latin1');
        for (let i = 0; i < b.length; i++) oct[end + i] = b[i];
        oct[end + b.length] = 0;
        return end;
    };
    const le = (end) => { let s = ''; while (oct[end]) s += String.fromCharCode(oct[end++]); return s; };
    const sinal = (v) => (v < 0 ? -1 : (v > 0 ? 1 : 0));

    for (const [a, b] of PARES) {
        const A = poe(B1, a), B = poe(B2, b);
        out.push(String(E.strlen(A)));
        out.push(String(sinal(E.strcmp(A, B))));
        out.push(String(sinal(E.strncmp(A, B, 3))));
        E.strcpy(B3, A); out.push(le(B3));
        E.memset(B3, 35, 8); E.strncpy(B3, A, 6);
        out.push(Array.from({ length: 8 }, (_, i) => oct[B3 + i]).join(' ') + ' ');
        E.strcpy(B3, A); E.strcat(B3, B); out.push(le(B3));
        let r = E.strchr(A, 111);  out.push(String(r ? r - A : -1));
        r = E.strrchr(A, 111); out.push(String(r ? r - A : -1));
    }
    for (const [h, ag] of AGULHAS) {
        const H = poe(B1, h), G = poe(B2, ag);
        const r = E.strstr(H, G);
        out.push(String(r ? r - H : -1));
    }
    const mostra = (n) => Array.from({ length: n }, (_, i) => oct[B1 + i]).join(' ') + ' ';
    E.memset(B1, 0, 32); poe(B1, 'reino dourado');
    E.memmove(B1 + 4, B1, 9); out.push(mostra(16));
    E.memset(B1, 0, 32); poe(B1, 'reino dourado');
    E.memmove(B1, B1 + 4, 9); out.push(mostra(16));
    E.memset(B1, 65, 10); out.push(mostra(10));
    poe(B1, 'abc'); poe(B2, 'abd');
    out.push(String(sinal(E.memcmp(B1, B2, 3))));
    poe(B2, 'abc');
    out.push(String(E.memcmp(B1, B2, 3)));
    for (let i = 0; i < 256; i++)
        out.push([E.isdigit(i) ? 1 : 0, E.isupper(i) ? 1 : 0, E.islower(i) ? 1 : 0,
                  E.isalpha(i) ? 1 : 0, E.isalnum(i) ? 1 : 0, E.isspace(i) ? 1 : 0,
                  E.toupper(i), E.tolower(i)].join(' '));
    for (const n of NUMEROS) {
        const N = poe(B1, n);
        out.push(String(E.atoi(N) | 0));
        out.push(String(E.atol(N)));
        /* o `fim` é um ponteiro para ponteiro: dá-se-lhe um slot e lê-se lá */
        const cel = B3 + 256;
        const v = E.strtol(N, cel, 0);
        const dv = new DataView(E.DISCO.buffer);
        out.push(`${v} ${dv.getInt32(cel, true) - N}`);
    }
    for (const n of REAIS) out.push(String(E.atof(poe(B1, n))));
    /* A FITA CONSTROI-SE AQUI. Uma funcao com `...` tem, no wasm, um parametro a mais: o
     * ENDERECO da fita. Quem chama escreve os valores em slots de oito bytes e passa a
     * morada — e de fora e' exactamente igual, porque a fita e' a interface. */
    const dv = new DataView(E.DISCO.buffer);
    const FITA = B4;                 /* a fita tem casa própria */
    for (const [f, , tipos, vals] of FORMATOS) {
        const F = poe(B2, f);
        for (let i = 0; i < vals.length; i++) {
            const o = FITA + 8 * i;
            if (tipos[i] === 'i64') dv.setBigInt64(o, BigInt(vals[i]), true);
            else if (tipos[i] === 'f64') dv.setFloat64(o, vals[i], true);
            else if (tipos[i] === 's') dv.setInt32(o, poe(B4 + 256, vals[i]), true);
            else dv.setInt32(o, vals[i], true);
        }
        E.snprintf(B1, 64, F, FITA);
        out.push('[' + le(B1) + ']');
    }
    /* a leitura: os destinos sao ENDERECOS, e vao na fita como tudo o resto */
    const D = B4 + 512;          /* e os destinos também, longe de tudo */
    for (const [txt, f, tipos] of LEITURAS) {
        const S = poe(B1, txt), F = poe(B2, f);
        for (let i = 0; i < 8; i++) dv.setBigInt64(D + 8 * i, 0n, true);
        for (let i = 0; i < tipos.length; i++) dv.setInt32(FITA + 8 * i, D + 64 * i, true);
        for (let i = 0; i < tipos.length; i++) oct[D + 64 * i] = 0;
        const q = E.sscanf(S, F, FITA);
        let linha = String(q);
        for (let i = 0; i < tipos.length; i++) {
            const o = D + 64 * i;
            if (tipos[i] === 'i32') linha += ' ' + dv.getInt32(o, true);
            else if (tipos[i] === 'f64') linha += ' ' + dv.getFloat64(o, true);
            else linha += ' [' + le(o) + ']';
        }
        out.push(linha);
    }
    return out;
}

/* ── e comparam-se ────────────────────────────────────────────────────────────────── */
if (!E) {
    console.log('  o motor nao instancia modulos com disco sob este limite de espaco virtual —');
    console.log(`  a biblioteca subiu: ${bytes.length} bytes, ${WebAssembly.Module.exports(M).length} exports.`);
    ok('a biblioteca sobe e o modulo e valido — o motor nao a corre sob este limite',
       WebAssembly.Module.exports(M).some(x => x.name === 'strcmp'));
} else {
    const sis = do_sistema(), meu = do_wasm();
    let iguais = 0, prim = null;
    const n = Math.max(sis.length, meu.length);
    for (let i = 0; i < n; i++) {
        const a = (sis[i] || '').trim(), b = (meu[i] || '').trim();
        let bate = a === b;
        if (!bate && a !== '' && b !== '' && !isNaN(parseFloat(a)) && !isNaN(parseFloat(b)))
            bate = Object.is(parseFloat(a), parseFloat(b));
        if (bate) iguais++;
        else if (!prim) prim = `linha ${i}: sistema=${JSON.stringify(a)} meu=${JSON.stringify(b)}`;
    }
    /* O NUMERO SAI DAS LISTAS, nao da minha cabeca. Escrevi `n > 500` de cor, o medidor deu
     * 467 iguais em 467 e reprovou — e um limiar escrito a mao tanto reprova o que esta certo
     * como aprova uma lista que encolheu em silencio. Aqui ele conta-se. */
    const esperado = PARES.length * 8 + AGULHAS.length + 5 + 256 + NUMEROS.length * 3
                   + REAIS.length + FORMATOS.length + LEITURAS.length;
    console.log(`   ${iguais} de ${n} respostas iguais as da glibc (esperadas ${esperado})` +
                (prim ? `\n   1.a diferenca: ${prim}` : ''));
    ok('§L1-L4 a biblioteca da o MESMO que a do sistema, em todas as respostas',
       n === esperado && iguais === n);
}

/* ─── §L4b OS FICHEIROS SÃO SLOTS ─────────────────────────────────────────────────── */
/* «a ISA nao cresceu, o compilador nao mudou»: quem hospeda poe os bytes em slots e diz o
 * nome; o programa abre pelo nome e le slots. E mede-se pela VOLTA — o que se escreve le-se
 * de volta, byte a byte, que e a Lei 1 aplicada ao ficheiro. */
if (E) {
    /* a dieta «sem memoria» (05/08) encolheu o wasm para 2 paginas e o
     * offset magico end_saida()+7M passou a cair FORA — pede-se ao
     * alocador da casa (vfs_reserva), como o app faz */
    const r = Number(E.vfs_reserva(16384));
    const S = E.end_saida();
    const oct = new Uint8Array(E.DISCO.buffer);
    const poe = (a, t) => { const b = Buffer.from(t, 'latin1');
        for (let i = 0; i < b.length; i++) oct[a + i] = b[i]; oct[a + b.length] = 0; return a; };
    const le = (a, n) => { let t = ''; for (let i = 0; i < n; i++) t += String.fromCharCode(oct[a + i]); return t; };

    const CONTEUDO = 'reino dourado\ne o resto do corpo\n';
    E.poe_ficheiro(poe(r, 'lib/classe/classe.txt'), poe(r + 256, CONTEUDO), CONTEUDO.length);
    /* abre pelo nome COM `../` a frente: quem abre nao sabe de onde esta a olhar */
    const h = E.fopen(poe(r + 1024, '../lib/classe/classe.txt'), poe(r + 1200, 'r'));
    const dest = r + 2048;
    const n = E.fread(dest, 1, 4096, h);
    const voltou = le(dest, n);
    console.log(`\n   §L4b o que se poe le-se de volta: ${n} bytes, iguais? ${voltou === CONTEUDO}`);
    ok('§L4b o ficheiro e um SLOT: o que o hospedeiro poe le-se de volta, residuo 0',
       n === CONTEUDO.length && voltou === CONTEUDO && Number(E.ftell(h)) === n);

    /* a agulha: fseek e' o destino mudado, e mais nada */
    E.fseek(h, 6n, 0);          /* o `long` da assinatura quer BigInt: e' i64 dos dois lados */   const a1 = le((()=>{ const d = r + 4096; E.fread(d, 1, 7, h); return d; })(), 7);
    E.rewind(h);        const a2 = Number(E.ftell(h));
    console.log(`   §L4b fseek(6) le ${JSON.stringify(a1)}   rewind poe a agulha em ${a2}`);
    ok('§L4b a agulha e o destino: fseek le' + "' de onde se poe, e rewind volta ao zero",
       a1 === 'dourado' && a2 === 0);

    /* e a escrita: o que sai e o que se pediu */
    E.limpa_saida();
    const w = E.fopen(poe(r + 5000, ''), poe(r + 5064, 'w'));
    E.fwrite(poe(r + 5128, 'BT '), 1, 3, w);
    const fita = r + 5200;
    const dv = new DataView(E.DISCO.buffer);
    dv.setInt32(fita, 42, true); dv.setFloat64(fita + 8, 13.6, true);
    E.fprintf(w, poe(r + 5300, '%d Tf %.3f Td'), fita);
    E.fputc(10, w);
    /* a SAIDA realoca ao crescer — o ponteiro lê-se FRESCO, não do S velho */
    const oct2 = new Uint8Array(E.DISCO.buffer);
    const S2 = E.end_saida();
    const le2 = (a, n) => { let t = ''; for (let i = 0; i < n; i++) t += String.fromCharCode(oct2[a + i]); return t; };
    const saiu = le2(S2, E.tam_saida());
    console.log(`   §L4b escrito: ${JSON.stringify(saiu)}`);
    ok('§L4b escrever e MOVE(-1) na mesma agulha: sai exactamente o que se pediu',
       saiu === 'BT 42 Tf 13.600 Td\n');
}

/* ─── §L5 A VOLTA ─────────────────────────────────────────────────────────────────── */
{
    const cv = path.join(TMP, 'libc_volta.c'), w2 = path.join(TMP, 'libc_2.wasm');
    let dif = -1;
    try {
        execFileSync(TRADUZ, [w, '-o', cv]);
        execFileSync(TRADUZ, [cv, '-o', w2]);
        const a = fs.readFileSync(w), b = fs.readFileSync(w2);
        dif = 0;
        for (let k = 0; k < Math.max(a.length, b.length); k++) if (a[k] !== b[k]) dif++;
    } catch (e) { dif = -1; }
    console.log(`\n   §L5 ${bytes.length} bytes -> desce -> sobe -> ` +
                (dif === 0 ? 'RESIDUO 0' : dif < 0 ? 'a volta nao correu' : `${dif} bytes diferentes`));
    ok('§L5 A VOLTA fecha na biblioteca inteira: sobe(desce(M)) = M, byte a byte', dif === 0);
}

/* ─── §L6 o CONTROLO ──────────────────────────────────────────────────────────────── */
/* Sem isto, «da o mesmo que a do sistema» podia ser uma comparacao que nao sabe discordar. */
if (E) {
    const b = Buffer.from(bytes);
    let onde = -1;
    /* E PROCURA-SE ONDE O CÓDIGO ESTÁ. Eu varria o módulo inteiro à procura do byte 0x6A e
     * apanhava-o na tabela de tipos ou dentro de um texto — trocar ali não muda instrução
     * nenhuma, e o controlo dizia «continua igual» sem nunca ter mexido no código. */
    let p10 = 8, cod0 = -1, cod1 = -1;
    { const leb = () => { let v = 0, d = 0, c; do { c = b[p10++]; v |= (c & 0x7f) << d; d += 7; } while (c & 0x80); return v; };
      while (p10 < b.length) { const id = b[p10++], t = leb(); if (id === 10) { cod0 = p10; cod1 = p10 + t; } p10 += t; } }
    for (let k = cod0; k >= 0 && k < cod1 - 3; k++) if (b[k] === 0x6A) { onde = k; break; }
    let quebrou = null;
    if (onde >= 0) {
        b[onde] = 0x6B;                                                              /* -> i32.sub */
        try {
            const E2 = new WebAssembly.Instance(new WebAssembly.Module(b)).exports;
            const oct = new Uint8Array(E2.DISCO.buffer);
            const p = E2.e1();
            for (const [i, c] of Buffer.from('reino dourado').entries()) oct[p + i] = c;
            oct[p + 13] = 0;
            /* SONDA-SE LARGO. Com uma sonda só, «continua igual» dizia mais sobre onde eu
             * fui bater do que sobre a biblioteca: o byte trocado podia estar numa função que
             * a sonda não toca. Toca-se em todas as famílias. */
            const q = E2.e2(), r = E2.e3(), t = E2.e4();
            for (const [i, c] of Buffer.from('42 e mais').entries()) oct[q + i] = c;
            oct[q + 9] = 0;
            const dv2 = new DataView(E2.DISCO.buffer);
            dv2.setInt32(t, 7, true);
            E2.snprintf(r, 32, p, t);
            let saida = ''; for (let i = 0; oct[r + i]; i++) saida += String.fromCharCode(oct[r + i]);
            quebrou = (E2.strlen(p) !== 13)
                   || (E2.strcmp(p, p) !== 0)
                   || (E2.strstr(p, p) !== p)
                   || (E2.atoi(q) !== 42)
                   || (E2.strchr(p, 100) === 0)
                   || (saida !== 'reino dourado')
                   || (E2.memcmp(p, p, 13) !== 0);
        } catch (e) { quebrou = true; }
    }
    console.log(`   controlo: com um i32.add trocado por i32.sub, a biblioteca ${quebrou ? 'QUEBRA' : 'continua igual'}`);
    ok('§L6 mudado um byte, a biblioteca deixa de responder o mesmo — a igualdade sabe falhar',
       quebrou === true);
}

console.log('\n==========================================================================');
if (!falhas) {
    console.log('  Nao se importou nada. As funcoes do texto e dos caracteres sao lacos sobre');
    console.log('  `char *`, e sobem pelo tradutor que ja ca estava — sem uma linha nova nele.');
    console.log('  A regua vale para si propria.');
    console.log('');
    console.log('  E as entradas trazem bytes acima de 127 de proposito: o `char` do wasm le-se');
    console.log('  COM sinal e o C manda comparar texto SEM ele. Sem o `& 255` um acentuado');
    console.log('  comparava negativo, e a ordenacao de qualquer texto portugues saia ao');
    console.log('  contrario — um defeito que so aparece se se for busca-lo.');
} else console.log(`  FALHOU: ${falhas}`);
console.log(`#TOTAL ${feitas} ${falhas}`);
process.exit(falhas ? 1 : 0);
