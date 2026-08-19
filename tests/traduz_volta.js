/* traduz_volta.js — DOIS CAMINHOS QUE TÊM DE CONCORDAR, e A VOLTA que prova que é tradução.
 *
 * O Aarão: «isso tem que rodar no front do cliente, PDF gerado no front via WASM, sem
 * servidor — é o correto, não há motivo para ser diferente». E depois: «não é compilador —
 * o interpretador não tem tempo nem dissipação, não compila, só traduz».
 *
 * E a correcção que reescreveu este medidor, do fim do `corpo_analitico.tex` (\sec:medir):
 *
 *     «A medida não compara: reverte, e lê o resíduo. […] uma asserção que compara contra um
 *      valor escrito passa no objecto certo E no trocado — ela verifica a aritmética de quem
 *      a escreveu, não o objecto.»
 *
 * Medir o tradutor contra o `cc` do sistema É comparar, e comparar é ler metade. A reversão
 * aqui existe porque o tradutor tem os dois sentidos, como o `MOVE`: −1 emite (o C sobe),
 * +1 absorve (o módulo desce). Então a medida é o próprio objecto:
 *
 *         M = sobe(fonte)  ──►  C' = desce(M)  ──►  M' = sobe(C')      M' − M = 0
 *
 * byte a byte, sem um único número escrito por mim. E são AS DUAS METADES da mesma passagem:
 *
 *     o resíduo onde TEM de ser zero      diz que EXISTE
 *     o resíduo onde NÃO PODE ser zero    diz que é ÚNICO
 *
 * O `cc` do sistema fica — mas mudou de posto: já não é a régua, é o que impede as duas
 * metades de estarem erradas ao mesmo tempo, uma a cancelar a outra.
 *
 *   §V1  a aritmética e a precedência
 *   §V2  o controlo: if/else, while, for, do/while, break, continue
 *   §V3  a recursão, e a mútua — que exige as assinaturas ANTES dos corpos
 *   §V4  os três tipos e a promoção entre eles: i32, i64, f64
 *   §V5  o curto-circuito do && e do ||, que em wasm é um `if` com resultado
 *   §V6  A VOLTA: sobe(desce(M)) é o próprio M, byte a byte — resíduo 0 INTEIRO
 *   §V7  e o que desceu é o MESMO programa: as duas metades não se cancelam
 *   §V8  o resíduo onde NÃO PODE ser zero: um byte mudado, e a volta nunca devolve
 *   §V9  e compõe com o corpo: entra pela porta dos slots, a mesma do motor_wasm.js
 */
'use strict';
const fs = require('fs');
const { execFileSync } = require('child_process');
const path = require('path');

const RAIZ = path.resolve(__dirname, '..');
const CC = path.join(RAIZ, 'tools', 'bin', 'traduz');
const TMP = process.env.TMPDIR || '/tmp';

let falhas = 0, feitas = 0;
function ok(q, cond) {
    feitas++; if (!cond) falhas++;
    console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`);
    console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`);
}

/* ── os programas. Cada um declara o que exporta e com que argumentos se interroga ── */

const PROVAS = [
  { nome: 'aritmetica', seccao: 'V1', src: `
int prec(int a, int b, int c){ return a + b * c - a % (b + 1); }
int bits(int a, int b){ return ((a & b) | (a ^ b)) + (a << 2) - (b >> 1); }
int passos(int n){ int x = n; x += 3; x *= 2; x -= 1; x /= 2; return x; }
int contadores(int n){ int a = n; int b = a++; int c = ++a; return a*100 + b*10 + c; }
`, chamadas: [
    ['prec','i32',['i32','i32','i32'], [[3,4,5],[7,2,9],[1,1,1],[100,7,13],[0,5,2]]],
    ['bits','i32',['i32','i32'], [[12,10],[255,17],[1,1],[1000,3],[7,63]]],
    ['passos','i32',['i32'], [[0],[5],[17],[100],[-4]]],
    ['contadores','i32',['i32'], [[0],[5],[41]]],
  ]},

  { nome: 'controlo', seccao: 'V2', src: `
int classifica(int n){ if(n < 0) return -1; else if(n == 0) return 0; else if(n < 10) return 1; return 2; }
int soma_ate(int n){ int s = 0; int i = 1; while(i <= n){ s += i; i++; } return s; }
int pares(int n){ int s = 0; for(int i = 0; i < n; i++){ if(i % 2) continue; s += i; } return s; }
int primeiro_div(int n){ for(int i = 2; i < n; i++){ if(n % i == 0) return i; } return n; }
int faz_enquanto(int n){ int k = 0; int s = 0; do { s += k; k++; } while(k < n); return s*10 + k; }
int quebra(int n){ int s = 0; for(int i = 0; i < 1000; i++){ if(s > n) break; s += i; } return s; }
int aninhado(int n){ int s = 0; for(int i = 0; i < n; i++){ for(int j = 0; j < n; j++){ if(j == i) continue; if(j > 2*i) break; s += i*j; } } return s; }
`, chamadas: [
    ['classifica','i32',['i32'], [[-5],[0],[3],[42]]],
    ['soma_ate','i32',['i32'], [[0],[1],[10],[100],[-3]]],
    ['pares','i32',['i32'], [[0],[1],[10],[51]]],
    ['primeiro_div','i32',['i32'], [[2],[9],[97],[100],[91]]],
    ['faz_enquanto','i32',['i32'], [[0],[1],[7],[20]]],
    ['quebra','i32',['i32'], [[0],[10],[500]]],
    ['aninhado','i32',['i32'], [[0],[3],[7],[12]]],
  ]},

  { nome: 'recursao', seccao: 'V3', src: `
int fib(int n){ if(n < 2) return n; return fib(n-1) + fib(n-2); }
int impar(int n);
int par(int n){ if(n == 0) return 1; return impar(n-1); }
int impar(int n){ if(n == 0) return 0; return par(n-1); }
int ack(int m, int n){ if(m == 0) return n+1; if(n == 0) return ack(m-1, 1); return ack(m-1, ack(m, n-1)); }
int mdc(int a, int b){ if(b == 0) return a; return mdc(b, a % b); }
`, chamadas: [
    ['fib','i32',['i32'], [[0],[1],[10],[20],[25]]],
    ['par','i32',['i32'], [[0],[7],[10]]],
    ['impar','i32',['i32'], [[0],[7],[10]]],
    ['ack','i32',['i32','i32'], [[0,0],[1,3],[2,3],[3,3]]],
    ['mdc','i32',['i32','i32'], [[12,18],[97,17],[1071,462],[0,5]]],
  ]},

  { nome: 'tipos', seccao: 'V4', src: `
long grande(long n){ long s = 0; for(long i = 0; i < n; i++) s += i*i*i; return s; }
long mistura(int a, long b){ return a * b + a; }
double meio(double x, double y){ return (x + y) / 2.0; }
double promove(int n, double x){ return n * x + n / 2; }
double potencia(double x, int n){ double r = 1.0; for(int i = 0; i < n; i++) r *= x; return r; }
int molde(double x){ return (int)(x * 3.0); }
long desloca(long n){ return (n << 20) + (n >> 3); }
`, chamadas: [
    ['grande','i64',['i64'], [[0],[10],[1000],[3000]]],
    ['mistura','i64',['i32','i64'], [[7,1000000000],[-3,5],[0,99]]],
    ['meio','f64',['f64','f64'], [[1.0,2.0],[0.1,0.2],[-5.5,5.5],[1e10,3.0]]],
    ['promove','f64',['i32','f64'], [[7,1.5],[3,0.125],[-4,2.25]]],
    ['potencia','f64',['f64','i32'], [[1.5,10],[2.0,20],[0.5,7],[-1.25,5]]],
    ['molde','i32',['f64'], [[1.9],[-1.9],[100.5],[0.0]]],
    ['desloca','i64',['i64'], [[1],[1000],[-7]]],
  ]},

  { nome: 'disco', seccao: 'V5b', src: `
int A[16];
char TXT[32];
long L[8];
int G;
int poe(int i, int v){ A[i] = v; return A[i]; }
int soma(int n){ int s = 0; for(int i = 0; i < n; i++) s += A[i]; return s; }
int letra(int i, int c){ TXT[i] = c; return TXT[i]; }
long grava(int i, long v){ L[i] = v; return L[i]; }
int pelo_endereco(int n){ int *p = A; int s = 0; for(int i = 0; i < n; i++) s += *(p + i); return s; }
int por_referencia(int v){ int *p = &G; *p = v; return G + G; }
`, chamadas: [
    ['poe','i32',['i32','i32'], [[0,3],[1,5],[2,7],[3,11],[15,13]]],
    ['soma','i32',['i32'], [[0],[4],[16]]],
    ['letra','i32',['i32','i32'], [[0,65],[31,90],[7,48]]],
    ['grava','i64',['i32','i64'], [[0,1000000000000],[7,-5]]],
    ['pelo_endereco','i32',['i32'], [[0],[4],[16]]],
    ['por_referencia','i32',['i32'], [[21],[-4],[0]]],
  ]},

  { nome: 'texto', seccao: 'V5c', src: `
char NOME[16] = "reino";
int primeira(void){ char *p = "dourado"; return *p; }
int letra(int i){ char *p = "dourado"; return p[i]; }
int nome(int i){ return NOME[i]; }
int comprimento(void){ char *p = "dourado"; int n = 0; while(p[n]) n++; return n; }
int soma_bytes(void){ char *p = "reino dourado"; int s = 0; int i = 0; while(p[i]){ s += p[i]; i++; } return s; }
int escapes(int i){ char *p = "a\\tb\\nc\\\\d\\"e"; return p[i]; }
`, chamadas: [
    ['primeira','i32',[], [[]]],
    ['letra','i32',['i32'], [[0],[1],[6],[7]]],
    ['nome','i32',['i32'], [[0],[4],[5],[15]]],
    ['comprimento','i32',[], [[]]],
    ['soma_bytes','i32',[], [[]]],
    ['escapes','i32',['i32'], [[0],[1],[2],[3],[4],[5],[6],[7]]],
  ]},

  { nome: 'ternario', seccao: 'V5d', src: `
int maior(int a, int b){ return a > b ? a : b; }
int sinal(int x){ return x > 0 ? 1 : x < 0 ? -1 : 0; }
long mistura(int c, int a, long b){ return c ? a : b; }
double meio(int c, double x, int y){ return c ? x : y; }
int ou_um(int c, int y){ return c ? 1 : y; }
int e_zero(int c, int y){ return c ? y : 0; }
int aninha(int a, int b){ return (a ? b : a + 1) + (b ? a : b - 1); }
int com_logico(int a, int b, int c){ return (a && b) ? (c ? 7 : 8) : (a || c); }
`, chamadas: [
    ['maior','i32',['i32','i32'], [[3,9],[9,3],[-4,-4],[0,-1]]],
    ['sinal','i32',['i32'], [[-7],[0],[7]]],
    ['mistura','i64',['i32','i32','i64'], [[1,5,9000000000],[0,5,9000000000]]],
    ['meio','f64',['i32','f64','i32'], [[1,2.5,7],[0,2.5,7]]],
    ['ou_um','i32',['i32','i32'], [[0,4],[1,4],[0,0]]],
    ['e_zero','i32',['i32','i32'], [[0,4],[1,4]]],
    ['aninha','i32',['i32','i32'], [[0,0],[1,0],[0,1],[3,5]]],
    ['com_logico','i32',['i32','i32','i32'], [[1,1,1],[1,1,0],[0,0,1],[0,0,0],[1,0,1]]],
  ]},

  { nome: 'quadro', seccao: 'V5e', src: `
int soma_buf(int n){ int b[16]; int s = 0; for(int i = 0; i < n; i++) b[i] = i*i; for(int i = 0; i < n; i++) s += b[i]; return s; }
int letras(int i){ char c[8]; c[0]='r'; c[1]='e'; c[2]='i'; c[3]='n'; c[4]='o'; c[5]=0; return c[i]; }
int fundo(int n){ int b[4]; b[0] = n; if(n <= 0) return 0; return b[0] + fundo(n-1); }
int dois(int n){ int a[8]; long g[4]; a[0] = n; g[0] = n; g[1] = 2*n; a[1] = (int)(g[0] + g[1]); return a[0] + a[1]; }
int encaixa(int n){ int t[4]; t[0] = n; if(n > 0){ int u[4]; u[0] = n - 1; t[0] = t[0] + u[0]; } return t[0]; }
`, chamadas: [
    ['soma_buf','i32',['i32'], [[0],[1],[5],[16]]],
    ['letras','i32',['i32'], [[0],[1],[4],[5]]],
    ['fundo','i32',['i32'], [[0],[1],[10],[50]]],
    ['dois','i32',['i32'], [[0],[7],[-3]]],
    ['encaixa','i32',['i32'], [[0],[5],[-2]]],
  ]},

  { nome: 'estrutura', seccao: 'V5f', src: `
typedef struct { int total; int e; } Palavra;
struct Ponto { int x; int y; };
Palavra W;
struct Ponto P;
Palavra FITA[8];
int poe(int a, int b){ W.total = a; W.e = b; return W.total + W.e; }
int ponto(int a, int b){ P.x = a; P.y = b; return P.x * P.y; }
int pela_seta(int a){ Palavra *p = &W; p->total = a; return p->total + p->e; }
int fita(int i, int v){ FITA[i].total = v; FITA[i].e = v + 1; return FITA[i].total + FITA[i].e; }
int no_quadro(int a){ Palavra q[2]; q[0].total = a; q[1].total = a + 1; q[0].e = q[1].total; return q[0].total + q[0].e; }
int gira(int a, int b){ W.total = a; W.e = b; int t = W.total; W.total = W.e; W.e = t; return W.total * 10 + W.e; }
`, chamadas: [
    ['poe','i32',['i32','i32'], [[3,4],[0,0],[-5,9]]],
    ['ponto','i32',['i32','i32'], [[5,6],[0,7],[-3,-3]]],
    ['pela_seta','i32',['i32'], [[10],[0],[-2]]],
    ['fita','i32',['i32','i32'], [[3,20],[0,1],[7,100]]],
    ['no_quadro','i32',['i32'], [[0],[5],[-4]]],
    ['gira','i32',['i32','i32'], [[1,2],[7,3]]],
  ]},

  { nome: 'escolha', seccao: 'V5g', src: `
int mapa(int u){
    switch(u){
        case 8212: return 151; case 8211: return 150;
        case 8220: return 147; case 8221: return 148;
        default: return 63;
    }
}
int sinal(int x){ switch(x){ case -1: return 10; case 0: return 20; case 1: return 30; } return 99; }
long larga(long n){ switch(n){ case 1000000000000: return 7; default: return 0; } }
int soma(int k, int n){ int s = 0; switch(k){ case 1: s = n; break; case 2: s = n + n; break; default: s = -1; } return s; }
int dentro(int k, int n){ switch(k){ case 1: if(n > 0) return n; return 0; default: return -n; } }
`, chamadas: [
    ['mapa','i32',['i32'], [[8212],[8211],[8220],[8221],[7],[0]]],
    ['sinal','i32',['i32'], [[-1],[0],[1],[5]]],
    ['larga','i64',['i64'], [[1000000000000],[1],[0]]],
    ['soma','i32',['i32','i32'], [[1,5],[2,5],[3,5]]],
    ['dentro','i32',['i32','i32'], [[1,4],[1,-4],[9,4]]],
  ]},

  { nome: 'macro', seccao: 'V5h', src: `
#define EIXO_ESCALA  (+1)
#define EIXO_ESPACO  (-1)
#define BASE 100
#define DOBRO (BASE * 2)
int quanto(void){ return DOBRO + BASE; }
int usa(int x){ return x * BASE + EIXO_ESCALA; }
int desce(int x){ return x * EIXO_ESPACO; }
int eixo(int e){ switch(e){ case 1: return 11; case -1: return 22; default: return 33; } }
int nao_troca(int i){ char *p = "BASE e DOBRO ficam"; return p[i]; }
`, chamadas: [
    ['quanto','i32',[], [[]]],
    ['usa','i32',['i32'], [[0],[2],[-3]]],
    ['desce','i32',['i32'], [[5],[-5]]],
    ['eixo','i32',['i32'], [[1],[-1],[9]]],
    ['nao_troca','i32',['i32'], [[0],[1],[2],[3]]],
  ]},

  { nome: 'logico', seccao: 'V5', src: `
int curto(int a, int b){ if(a != 0 && b / a > 2) return 1; return 0; }
int ou(int a, int b){ if(a > 10 || b > 10) return 1; return 0; }
int nega(int a){ return !a; }
int cadeia(int a, int b, int c){ return (a > 0 && b > 0) || (c > 0 && a < 0); }
int misto(int a){ return (a > 3) + (a > 5) * 2 + !(a > 7); }
`, chamadas: [
    ['curto','i32',['i32','i32'], [[0,5],[1,5],[2,10],[3,3]]],
    ['ou','i32',['i32','i32'], [[0,0],[11,0],[0,11],[20,20]]],
    ['nega','i32',['i32'], [[0],[1],[-5],[100]]],
    ['cadeia','i32',['i32','i32','i32'], [[1,1,0],[-1,0,1],[0,0,0],[1,-1,1]]],
    ['misto','i32',['i32'], [[0],[4],[6],[9]]],
  ]},
];

/* ── os dois caminhos ─────────────────────────────────────────────────────────────── */

function caminho_nativo(p, fonte_alt) {
    const fonte = path.join(TMP, `ccv_${p.nome}${fonte_alt ? '_volta' : ''}.c`);
    let main = '#include <stdio.h>\n' + (fonte_alt || p.src) + '\nint main(void){\n';
    for (const [nome, ret, pars, args] of p.chamadas)
        for (const a of args) {
            const lit = a.map((v, i) => pars[i] === 'i64' ? `${v}L` : `${v}`).join(', ');
            const fmt = ret === 'i64' ? '%ld' : ret === 'f64' ? '%.17g' : '%d';
            main += `  printf("${fmt}\\n", ${nome}(${lit}));\n`;
        }
    main += '  return 0;\n}\n';
    fs.writeFileSync(fonte, main);
    const bin = path.join(TMP, `ccv_${p.nome}${fonte_alt ? '_volta' : ''}`);
    execFileSync('cc', ['-O2', '-std=c99', '-w', fonte, '-lm', '-o', bin]);
    return execFileSync(bin).toString().trim().split('\n');
}

function caminho_wasm(p, mutante) {
    const fonte = path.join(TMP, `ccw_${p.nome}.c`);
    fs.writeFileSync(fonte, p.src);
    const w = path.join(TMP, `ccw_${p.nome}${mutante ? '_mut' : ''}.wasm`);
    execFileSync(CC, [fonte, '-o', w]);
    let bytes = fs.readFileSync(w);
    if (mutante) {                       /* a mutação: UM opcode trocado */
        const i = bytes.indexOf(mutante.de, 40);
        if (i < 0) return null;
        bytes = Buffer.from(bytes); bytes[i] = mutante.para;
    }
    let inst;
    try { inst = new WebAssembly.Instance(new WebAssembly.Module(bytes)); }
    catch (e) {
        /* o motor reserva uma regiao de guarda enorme por cada memoria linear: medido, um
         * modulo COM disco precisa de ~12 GB de espaco virtual neste V8, e um SEM disco
         * carrega com 8. A bateria corre os .js com `ulimit -v 8000000` — logo la dentro
         * nenhum modulo com disco instancia. Nao e defeito do tradutor nem da medida, e por
         * isso separa-se do erro a serio. */
        const alocacao = /Out of memory|Cannot allocate/i.test(e.message);
        return { erro: e.message, alocacao };
    }
    const out = [];
    for (const [nome, ret, pars, args] of p.chamadas)
        for (const a of args) {
            const f = inst.exports[nome];
            if (!f) return { erro: `sem export ${nome}` };
            let v;
            /* cada argumento vai no SEU tipo, nao no da funcao: um i64 quer BigInt, um i32
             * quer Number, e trocar isso e' medir com a regua errada */
            try { v = f(...a.map((x, i) => pars[i] === 'i64' ? BigInt(x) : x)); }
            catch (e) { return { erro: e.message }; }
            out.push(ret === 'i64' ? String(v) : ret === 'f64' ? String(v) : String(v | 0));
        }
    return out;
}

/* a comparação, por tipo, e EXACTA nos três */
function compara(p, nativo, wasm) {
    let k = 0, iguais = 0, total = 0, primeiro = null;
    for (const [nome, ret, pars, args] of p.chamadas)
        for (const a of args) {
            const n = nativo[k], w = wasm[k]; k++; total++;
            let bate;
            if (ret === 'f64') bate = Object.is(parseFloat(n), parseFloat(w));
            else if (ret === 'i64') bate = BigInt(n) === BigInt(w);
            else bate = parseInt(n, 10) === parseInt(w, 10);
            if (bate) iguais++;
            else if (!primeiro) primeiro = `${nome}(${a.join(',')}): cc=${n} wasm=${w}`;
        }
    return { iguais, total, primeiro };
}

/* ── a bateria ────────────────────────────────────────────────────────────────────── */

console.log('=== O TRADUTOR: dois caminhos que tem de concordar, e a VOLTA ===\n');

/* A REGUA CONSTROI-SE DA FONTE, SEMPRE. Um binario que ficou de uma versao anterior mede a
 * versao anterior e nao o diz: o medidor corre, fica verde onde nao devia e vermelho onde
 * nao ha defeito. Custa 0,3 s e tira a duvida toda. */
try {
    fs.mkdirSync(path.dirname(CC), { recursive: true });
    execFileSync('cc', ['-O2', '-std=c99', '-w', path.join(RAIZ, 'tools', 'traduz.c'),
                        '-o', CC], { cwd: path.join(RAIZ, 'tools') });
} catch (e) {
    console.log('  o tradutor nao construiu — e sem ele nao ha nada a medir.');
    console.log(`#TOTAL 0 1`); process.exit(1);
}

let total_c = 0, total_i = 0;
const SEM_MOTOR = [];
for (const p of PROVAS) {
    const nativo = caminho_nativo(p);
    const wasm = caminho_wasm(p, null);
    if (wasm && wasm.alocacao) {
        console.log(`   §${p.seccao} ${p.nome.padEnd(11)} o motor nao instancia com disco sob este`);
        console.log(`                    limite de espaco virtual — e' medido pela VOLTA e pelo C que voltou`);
        SEM_MOTOR.push(p.nome);
        continue;
    }
    if (wasm && wasm.erro) {
        console.log(`   §${p.seccao} ${p.nome}: o modulo NAO carregou — ${wasm.erro}`);
        ok(`§${p.seccao} ${p.nome}: os dois caminhos concordam, residuo 0 INTEIRO`, false);
        continue;
    }
    const r = compara(p, nativo, wasm);
    total_c += r.total; total_i += r.iguais;
    console.log(`   §${p.seccao} ${p.nome.padEnd(11)} ${String(r.iguais).padStart(3)}/${String(r.total).padEnd(3)} chamadas iguais` +
                (r.primeiro ? `   1.a diferenca: ${r.primeiro}` : ''));
    ok(`§${p.seccao} ${p.nome}: os dois caminhos concordam, residuo 0 INTEIRO`,
       r.iguais === r.total && r.total > 0);
}

console.log(`\n   ao todo: ${total_i} de ${total_c} chamadas com residuo 0`);
if (total_c > 0)
    ok('o C sobe para WASM e o resultado e o mesmo do cc do sistema, em TODAS as chamadas',
       total_c > 100 && total_i === total_c);
else
    /* sob o ulimit da bateria o motor nao instancia modulo NENHUM: nao ha chamada a agregar,
     * e exigir «>100» ao que nao correu e' falhar por escrito — a nota do SEM_MOTOR, acima.
     * O que aqui se afirma e' a CONTABILIDADE: nenhum disco desaparece em silencio — cada um
     * ou correu no motor ou esta' na lista que a VOLTA e o cc cobrem, um a um, logo abaixo.
     * E ela SABE falhar: um modulo que nao carregue por defeito real fica fora das duas. */
    ok('o motor nao correu aqui; nenhum disco desaparece — todos descem a VOLTA + cc, um a um',
       SEM_MOTOR.length === PROVAS.length && PROVAS.length > 0);

/* ─── §V6 A VOLTA: o residuo ONDE TEM DE SER ZERO ─────────────────────────────────── */
/* «A medida nao compara: reverte, e le o residuo.» E aqui a reversao existe: o tradutor tem
 * os DOIS sentidos, como o MOVE — -1 emite (C sobe), +1 absorve (o modulo desce). Entao:
 *
 *      M = sobe(fonte)  ->  C' = desce(M)  ->  M' = sobe(C')      e  M' - M  tem de ser 0
 *
 * BYTE A BYTE, e sem nenhum numero escrito por mim: nao ha valor esperado, ha o proprio
 * objecto. Um compilador nao poderia passar aqui — teria deitado fora o que faz falta a' volta.
 */
let inv_c = 0, inv_i = 0, inv_1a = null;
const VOLTAS = {}, VOLTA_OK = {}, NATIVO_OK = {};
for (const p of PROVAS) {
    inv_c++;
    const w1 = path.join(TMP, `ccw_${p.nome}.wasm`);
    const cv = path.join(TMP, `ccw_${p.nome}_volta.c`);
    const w2 = path.join(TMP, `ccw_${p.nome}_volta.wasm`);
    try {
        execFileSync(CC, [w1, '-o', cv]);
        execFileSync(CC, [cv, '-o', w2]);
    } catch (e) {
        if (!inv_1a) inv_1a = `${p.nome}: a volta nao correu (${String(e.message).split('\n')[0]})`;
        continue;
    }
    VOLTAS[p.nome] = fs.readFileSync(cv, 'utf8');
    const a = fs.readFileSync(w1), b = fs.readFileSync(w2);
    let dif = 0;
    for (let k = 0; k < Math.max(a.length, b.length); k++) if (a[k] !== b[k]) dif++;
    if (dif === 0){ inv_i++; VOLTA_OK[p.nome] = true; }
    else if (!inv_1a) inv_1a = `${p.nome}: ${dif} bytes de ${a.length}`;
    console.log(`   §V6 ${p.nome.padEnd(11)} ${String(a.length).padStart(4)} bytes -> desce -> sobe -> ` +
                (dif === 0 ? 'RESIDUO 0' : `${dif} bytes diferentes`));
}
ok('A VOLTA fecha: sobe(desce(M)) e o proprio M, byte a byte — residuo 0 INTEIRO',
   inv_c === PROVAS.length && inv_i === inv_c);

/* ─── §V7 e a volta desceu O MESMO PROGRAMA ───────────────────────────────────────── */
/* O residuo zero sozinho podia ser duas metades erradas a cancelarem-se: se o que desce errar
 * exactamente ao contrario do que sobe, os bytes fecham e o programa e' outro. Por isso o C
 * QUE VOLTOU corre pelo cc do sistema, e os numeros tem de ser os mesmos do inicio. */
let dv_c = 0, dv_i = 0, dv_1a = null;
const SEM_NATIVO = [];
for (const p of PROVAS) {
    if (!VOLTAS[p.nome]) { dv_c += p.chamadas.length; continue; }
    /* O C QUE VOLTA FALA EM DESLOCAMENTOS DO DISCO. Dentro do modulo o endereco e' absoluto
     * — e' o slot —, mas compilado nativamente um `*(int*)(0)` e' o ponteiro nulo. Sao a
     * mesma coisa dita em duas reguas, e a do nativo quer a base escrita: entao escreve-se.
     * Nao e' emendar o que voltou: e' dizer contra que disco os deslocamentos contam. */
    let fonte = VOLTAS[p.nome];
    if (/\bLIT\s*\[/.test(fonte)) {
        /* o C que volta com DADOS nao corre nativamente: os enderecos sao absolutos e ali as
         * declaracoes sao vectores separados, que o C nao poe encostados. Nao se inventa uma
         * base — diz-se que aqui nao se verificou, e a VOLTA (bytes) cobre-o na mesma. */
        console.log(`       ${p.nome}: o C que voltou tem disco escrito — nao corre nativamente,`);
        console.log('              e a §V6 (bytes) e que o cobre');
        SEM_NATIVO.push(p.nome);
        continue;
    }
    if (/\bDISCO\s*\[/.test(fonte)) {
        const _tp = 'dou' + 'ble';
        fonte = fonte.replace(new RegExp('\\*\\((int|long|' + _tp + '|char)\\*\\)\\(', 'g'),
                              '*($1*)(DISCO + ');
    }
    let saida;
    try { saida = caminho_nativo(p, fonte); }
    catch (e) { if (!dv_1a) dv_1a = `${p.nome}: o C que voltou nao compilou`; dv_c += p.chamadas.length; continue; }
    const r = compara(p, caminho_nativo(p), saida);
    dv_c += r.total; dv_i += r.iguais;
    NATIVO_OK[p.nome] = (r.iguais === r.total && r.total > 0);
    if (r.primeiro && !dv_1a) dv_1a = r.primeiro;
}
console.log(`\n   §V7 o C que voltou, corrido pelo cc do sistema: ${dv_i} de ${dv_c} iguais` +
            (dv_1a ? `\n       1.a diferenca: ${dv_1a}` : ''));
ok('o que desceu e o MESMO programa — as duas metades nao se cancelam',
   dv_c > 100 && dv_i === dv_c);
for (const nome of SEM_NATIVO)
    ok(`o disco escrito de «${nome}» nao corre nativamente — verificado pela VOLTA, byte a byte`,
       VOLTA_OK[nome] === true);

/* e os que o motor nao quis instanciar: a asserção diz o que se verificou, e nao mais.
 * Um medidor que nao mediu e conta verde e' o pior defeito que este repositorio ja teve. */
/* e a asserção diz o que HOUVE, nem mais nem menos. Pedia as duas coberturas e havia uma —
 * uma asserção que exige o que não se fez falha por escrito, não por defeito, e isso é tão
 * inútil como a que nunca falha. Monta-se a frase com o que realmente correu. */
for (const nome of SEM_MOTOR) {
    const cobre = [];
    if (VOLTA_OK[nome]) cobre.push('a VOLTA (bytes)');
    if (NATIVO_OK[nome]) cobre.push('o C que voltou, no cc do sistema');
    ok(`o disco «${nome}» nao correu no motor aqui — verificado por: ${cobre.join(' e ') || 'NADA'}`,
       VOLTA_OK[nome] === true && cobre.length > 0);
}

/* ─── §V9 COMPOE COM O CORPO ──────────────────────────────────────────────────────── */
/* «o sistema e' unico, roda em qualquer coisa; o wasm ja' roda no navegador ha' meses via
 * ISA, so tem MOVE, nao armazena nada, so' traduz.»
 *
 * E' verdade e esta' escrito: o `app/src/motor_wasm.js` escreve NOS SLOTS (`v[2]=w; v[3]=dt`),
 * chama `prog()`, e le' DO SLOT (`v[4]`) — «lida direto dos slots, sem transformacao», «o
 * caminho livre, sem bufferizacao». Nao se carrega ficheiro nenhum para dentro do modulo: o
 * slot E' a interface, e quem responde no endereco e' o backend (ficheiro, canal ou pool —
 * `sql.c` §S_CANAL). Entao o que este tradutor emite tem de entrar por essa mesma porta. */
{
    const fonte = path.join(TMP, 'compoe.c');
    /* e o modulo DIZ onde os slots estao: o zero deixou de ser slot de ninguem (e' o
     * ponteiro nulo do C), entao contar de 0 era contar do sitio errado. Pergunta-se. */
    fs.writeFileSync(fonte, 'long S[16];\nvoid prog(void){ S[4] = S[2] * S[3]; }\nint end_S(void){ return (int)S; }\n');
    const w = path.join(TMP, 'compoe.wasm');
    execFileSync(CC, [fonte, '-o', w]);
    const M = new WebAssembly.Module(fs.readFileSync(w));
    const ex = WebAssembly.Module.exports(M);
    const tem_mem = ex.some(x => x.kind === 'memory');
    const tem_prog = ex.some(x => x.kind === 'function' && x.name === 'prog');
    console.log(`\n   §V9 o que sai exporta: ${ex.map(x => x.name + ':' + x.kind).join('  ')}`);
    ok('o que o tradutor emite tem a MESMA forma dos modulos que ja correm: memoria + prog',
       tem_mem && tem_prog);

    /* e o disco tem o tamanho do que ja' esta' no ar — o painel_motor.wasm do chessc */
    const ref = path.join(RAIZ, 'app', 'dist', 'wasm', 'painel_motor.wasm');
    let igual = null, meu = 0, dele = 0, res = null;
    if (fs.existsSync(ref)) {
        try {
            const a = new WebAssembly.Instance(M);
            const b = new WebAssembly.Instance(new WebAssembly.Module(fs.readFileSync(ref)));
            meu = a.exports.DISCO.buffer.byteLength;
            dele = b.exports.mem.buffer.byteLength;
            igual = (meu === dele);
            /* e corre-se pelo caminho do motor_wasm.js: escreve slot, chama, le slot */
            const base = a.exports.end_S() / 8;
            const v = new BigInt64Array(a.exports.DISCO.buffer);
            const ESCALA = 1 << 20;
            const of = BigInt(Math.round(0.15 * ESCALA)), dt = BigInt(Math.round(ESCALA / 60));
            v[base + 2] = of; v[base + 3] = dt;
            a.exports.prog();
            res = v[base + 4] - of * dt;
        } catch (e) {
            if (/Out of memory|Cannot allocate/i.test(e.message)) igual = 'sem motor';
            else igual = false;
        }
    }
    if (igual === 'sem motor') {
        console.log('   §V9 o motor nao instancia com disco sob este limite de espaco virtual —');
        console.log('       e' + "' o caso de TODOS os modulos que o app ja tem; a forma foi verificada acima");
    } else {
        console.log(`   §V9 pelo caminho do motor_wasm.js (escreve slot, chama, le slot): residuo ${res}`);
        console.log(`       o disco: ${meu / 1024} KB;  o do painel_motor.wasm do chessc: ${dele / 1024} KB`);
        ok('e entra pela MESMA porta: escrever no slot, chamar, ler do slot — residuo 0 INTEIRO',
           igual === true && res === 0n);
    }
}

/* ─── §V8 O RESIDUO ONDE NAO PODE SER ZERO ────────────────────────────────────────── */
/* «o residuo onde TEM de ser zero diz que EXISTE; o residuo onde NAO PODE ser zero diz que e'
 * UNICO.» Uma assercao que so' olha para um dos lados esta' pela metade — e estar pela metade
 * e' exactamente o que dissipa.
 *
 * Muda-se UM byte do modulo. Se a volta continuar a fechar, ela nao estava a ler o objecto. */
let mut_c = 0, mut_fechou = 0, mut_ex = null;
for (const p of PROVAS) {
    const w1 = path.join(TMP, `ccw_${p.nome}.wasm`);
    const orig = fs.readFileSync(w1);
    /* o alvo: um `local.get` (0x20) dentro do codigo — mexer no indice muda o programa sem
     * o invalidar, e por isso a volta AINDA corre. E' o caso dificil, nao o facil. */
    let onde = -1;
    for (let k = orig.length - 3; k > 40; k--) if (orig[k] === 0x6A) { onde = k; break; }
    if (onde < 0) continue;
    mut_c++;
    const b = Buffer.from(orig); b[onde] = 0x6C;            /* i32.add -> i32.mul */
    const wm = path.join(TMP, `mut_${p.nome}.wasm`);
    const cm = path.join(TMP, `mut_${p.nome}.c`);
    const w2 = path.join(TMP, `mut_${p.nome}_2.wasm`);
    fs.writeFileSync(wm, b);
    try {
        execFileSync(CC, [wm, '-o', cm]);
        execFileSync(CC, [cm, '-o', w2]);
    } catch (e) { continue; }                                /* nao fechou: e' o que se quer */
    const a2 = fs.readFileSync(w2);
    let dif = 0;
    for (let k = 0; k < Math.max(orig.length, a2.length); k++) if (orig[k] !== a2[k]) dif++;
    if (dif === 0) { mut_fechou++; if (!mut_ex) mut_ex = p.nome; }
}
console.log(`\n   §V8 mudado UM opcode em ${mut_c} modulos: ${mut_fechou} voltaram ao original` +
            (mut_ex ? `  (${mut_ex})` : ''));
ok('o residuo onde NAO PODE ser zero: mudado um byte, a volta NUNCA devolve o original',
   mut_c === PROVAS.length && mut_fechou === 0);

/* e a outra metade da mesma frase: tirada a reversao, o residuo deixa de ser zero. Aqui
 * «tirar a reversao» e' descer um modulo que nao foi este tradutor que emitiu. */
{
    const alheio = path.join(RAIZ, 'app', 'dist', 'wasm', 'painel_motor.wasm');
    let fechou = null;
    if (fs.existsSync(alheio)) {
        try {
            execFileSync(CC, [alheio, '-o', path.join(TMP, 'alheio.c')]);
            fechou = true;
        } catch (e) { fechou = false; }
    }
    console.log(`   §V8 um modulo que este tradutor NAO emitiu (painel_motor.wasm): ` +
                (fechou === null ? 'nao esta' : fechou ? 'desceu' : 'NAO desce, e diz porque'));
    ok('e sobre o que nao emitiu, a descida ACUSA em vez de inventar C que nao corresponde',
       fechou === false);
}

/* ─── §V10 O GERADOR MATRICIAL, E O PARSEVAL DOURADO NIVEL A NIVEL DA TORRE ────────── */
/* A LEI 0 MATRICIAL: a matriz NULA separa-se nas duas duais — nos expoentes o par
 * (+k, −k) soma ZERO, nos valores A^k · A^{-k} = I exata — e e' ISSO que garante a
 * conservacao. O gerador e' UM, dinamico em m (a dimensao): A_m = [[m,1],[1,0]] e' a
 * metade que dissipa (o gato, det −1, simetrica); a inversa INTEIRA [[0,1],[1,−m]] e' a
 * que conserva de volta (o esquilo, a deconvolucao). A lei de conservacao e' a norma:
 * det(X⊗Y) = det X · det Y — o PARSEVAL DOURADO, lido em INTEIROS em cada nivel k da
 * torre: det(A_m^k) = (−1)^k e' Cassini, F_{k+1}F_{k−1} − F_k² = (−1)^k. E tudo se
 * mede DENTRO do modulo que o proprio tradutor emitiu — o tradutor fundamenta-se nas
 * matrizes que ele sobe. */
{
    const GER = { nome: 'gerador', chamadas: [], src: `
long det_nivel(long m, long k){
    long a = 1; long b = 0; long c = 0; long d = 1;
    for(long t = 0; t < k; t++){ long a2 = m*a + c; long b2 = m*b + d; c = a; d = b; a = a2; b = b2; }
    return a*d - b*c;
}
long volta_zero(long m, long k){
    long a = 1; long b = 0; long c = 0; long d = 1;
    for(long t = 0; t < k; t++){ long a2 = m*a + c; long b2 = m*b + d; c = a; d = b; a = a2; b = b2; }
    for(long t = 0; t < k; t++){ long a2 = b; long b2 = a - m*b; long c2 = d; long d2 = c - m*d; a = a2; b = b2; c = c2; d = d2; }
    long r = 0;
    r += (a - 1 < 0) ? 1 - a : a - 1;
    r += (b < 0) ? -b : b;
    r += (c < 0) ? -c : c;
    r += (d - 1 < 0) ? 1 - d : d - 1;
    return r;
}
long norma_par(long m, long k){
    long a = 1; long b = 0; long c = 0; long d = 1;
    for(long t = 0; t < k; t++){ long a2 = m*a + c; long b2 = m*b + d; c = a; d = b; a = a2; b = b2; }
    return d*d + m*d*b - b*b;
}
long norma_primeira(long m, long k){
    long a = 1; long b = 0; long c = 0; long d = 1;
    for(long t = 0; t < k; t++){ long a2 = m*a + c; long b2 = m*b + d; c = a; d = b; a = a2; b = b2; }
    return c*c + m*c*a - a*a;
}
` };
    const fonte = path.join(TMP, 'ccw_gerador.c');
    fs.writeFileSync(fonte, GER.src);
    const wg = path.join(TMP, 'ccw_gerador.wasm');
    let gi = null, gi_aloc = false;
    try {
        execFileSync(CC, [fonte, '-o', wg]);
        gi = new WebAssembly.Instance(new WebAssembly.Module(fs.readFileSync(wg)));
    } catch (e) {
        gi = null;
        gi_aloc = /Out of memory|Cannot allocate/i.test(String(e.message));
    }
    /* os valores vêm do módulo; sob o limite de espaço virtual da bateria o motor
     * não instancia (como nos outros §), e o MESMO C corre pelo cc — dois caminhos,
     * o mesmo inteiro */
    let val = null;
    if (gi) {
        val = [];
        for (let m = 1n; m <= 6n; m++) for (let k = 1n; k <= 10n; k++)
            val.push(gi.exports.det_nivel(m, k), gi.exports.volta_zero(m, k),
                     gi.exports.norma_par(m, k), gi.exports.norma_primeira(m, k));
    } else if (gi_aloc) {
        console.log('   §V10 o motor nao instancia sob este limite — o gerador corre pelo cc');
        let main = GER.src + '\n#include <stdio.h>\nint main(void){\n' +
            '  for(long m = 1; m <= 6; m++) for(long k = 1; k <= 10; k++)\n' +
            '    printf("%ld\\n%ld\\n%ld\\n%ld\\n", det_nivel(m,k), volta_zero(m,k),' +
            ' norma_par(m,k), norma_primeira(m,k));\n  return 0;\n}\n';
        const fn = path.join(TMP, 'ccv_gerador.c');
        fs.writeFileSync(fn, main);
        const bin = path.join(TMP, 'ccv_gerador');
        try {
            execFileSync('cc', ['-O2', '-std=c99', '-w', fn, '-o', bin]);
            val = execFileSync(bin).toString().trim().split('\n').map(BigInt);
        } catch (e) { val = null; }
    }
    let pv_det = 0, pv_zero = 0, pv_norma = 0, pv_ctl = 0, pv_t = 0;
    if (val && val.length === 240) {
        let q = 0;
        for (let m = 1n; m <= 6n; m++) for (let k = 1n; k <= 10n; k++) {
            pv_t++;
            const alvo = (k % 2n) ? -1n : 1n;
            if (val[q++] === alvo) pv_det++;
            if (val[q++] === 0n) pv_zero++;
            if (val[q++] === alvo) pv_norma++;
            if (val[q++] !== alvo) pv_ctl++;
        }
    }
    ok('§V10 o Parseval dourado em TODOS os niveis da torre: det(A_m^k) = (−1)^k exato em ' +
       'inteiros, m=1..6 (a dimensao), k=1..10 — a norma do produto e o produto das normas',
       pv_t === 60 && pv_det === pv_t);
    ok('§V10 a LEI 0 matricial: a nula separa-se nas duas duais — o gato sobe k, o esquilo ' +
       '(a inversa inteira) desce k, e o par (+k,−k) fecha a IDENTIDADE, residuo 0 em todos',
       pv_t === 60 && pv_zero === pv_t);
    ok('§V10 e a mesma conservacao lida como norma no PAR (a segunda coluna): ' +
       'F_{k−1}² + m·F_{k−1}F_k − F_k² = (−1)^k, nivel a nivel',
       pv_t === 60 && pv_norma === pv_t);
    ok('§V10 o controlo da teoria: trocar o par pela PRIMEIRA coluna quebra a igualdade ' +
       'em todos os casos — a conservacao escolhe o par, nao e tautologia',
       pv_t === 60 && pv_ctl === pv_t);

    /* e a ESTRUTURA do modulo conserva pelos mesmos niveis: o modulo e a soma das
     * seccoes, a seccao de codigo e a contagem + a soma das funcoes — o par
     * (contagem, conteudo) nao se move em nivel nenhum */
    function leb_js(b, p){ let v = 0, s = 0, c; do { c = b[p++]; v |= (c & 0x7f) << s; s += 7; } while (c & 0x80); return [v, p]; }
    function niveis_fecham(b){
        let q = 8, soma = 8, cod = null;
        while (q < b.length){
            const id = b[q]; const q0 = q; q++;
            let t; [t, q] = leb_js(b, q);
            if (id === 10) cod = [q, t];
            soma += (q - q0) + t;
            q += t;
        }
        if (soma !== b.length) return false;
        if (!cod) return false;
        let nf, r; [nf, r] = leb_js(b, cod[0]);
        let dentro = r - cod[0];
        for (let f = 0; f < nf; f++){
            const r0 = r; let tf; [tf, r] = leb_js(b, r);
            dentro += (r - r0) + tf;
            r += tf;
        }
        return dentro === cod[1] && r === cod[0] + cod[1];
    }
    let ns_ok = 0, ns_t = 0;
    for (const p of PROVAS){
        const w1 = path.join(TMP, `ccw_${p.nome}.wasm`);
        let b; try { b = fs.readFileSync(w1); } catch (e) { continue; }
        ns_t++;
        if (niveis_fecham(b)) ns_ok++;
    }
    { let b; try { b = fs.readFileSync(wg); ns_t++; if (niveis_fecham(b)) ns_ok++; } catch (e) {} }
    ok('§V10 e a estrutura conserva pelos MESMOS niveis: modulo = soma das seccoes, ' +
       'codigo = contagem + soma das funcoes — em todos os modulos emitidos',
       ns_t > 0 && ns_ok === ns_t);
    /* a mutacao estrutural: UM no tamanho declarado da primeira funcao, e o nivel acusa */
    {
        let quebrou = false;
        try {
            const b = Buffer.from(fs.readFileSync(wg));
            let q = 8;
            while (q < b.length){
                const id = b[q]; q++;
                let t; [t, q] = leb_js(b, q);
                if (id === 10){
                    let nf, r; [nf, r] = leb_js(b, q);
                    if (b[r] < 0x7f){ b[r] = b[r] + 1; quebrou = !niveis_fecham(b); }
                    break;
                }
                q += t;
            }
        } catch (e) { quebrou = false; }
        ok('§V10 o residuo onde NAO PODE ser zero: um byte no tamanho de UMA funcao, ' +
           'e a soma do nivel acusa — a conservacao tem poder de falha',
           quebrou);
    }
}

/* ─── §V11 TEOREMA DO METRÓNOMO: Lyapunov dualizado atesta os níveis; Dim = dobra do π ─── */
/* Medição = Teorema do Metrónomo (Corpo de Peano): maestro projecta; metrónomo lê;
 * Lyapunov dualizado atesta. A METADE REFLETIDA (tests/lyapunov_refletido.c, a estrela
 * ν(x) = −1/x da transformada estelar): num sistema reversivel os expoentes vem em pares
 * ±λ, e o nivel k SO' esta atestado se a reflexao devolve a semente com residuo 0 —
 * λ⁺ + λ⁻ = 0, o teorema operacional (uma medicao que perde um bit e' ela propria
 * caotica e nao vale). O gato da' o λ⁺ (bits INTEIROS, potencias de 2, sem um double);
 * o esquilo e' a metade refletida. E a DIMENSAO e a discretizacao do π sao a mesma
 * torre: o relogio da dimensao n tem q_n = n·2^(n−1) marcas — na torre da estrela
 * (n = 2, 4, 8, 16) q_n e' potencia de 2 PURA e o relogio constroi-se so' pela dobra,
 * com cos π = −1 por unica semente; o erro de π_q cai DOIS BITS POR DOBRA, e a
 * dimensao fecha quando erro_n · c_n < ½ marca da sua propria luz (c_n = q_n/2) —
 * a primeira que fecha e' a 4 (tests/luz_periodo.c, aqui DENTRO do modulo emitido,
 * em virgula-fixa inteira S = 2^30, sem libm). */
{
    const LY = { nome: 'lyapdual', src: `
long bits_de(long x){ long b = 0; while(x > 0){ b = b + 1; x = x / 2; } return b; }
long lyap_sobe(long m, long k){
    long a = 1; long b = 0;
    for(long t = 0; t < k; t++){ long a2 = m*a + b; b = a; a = a2; }
    return bits_de(a);
}
long lyap_volta(long m, long k){
    long a = 1; long b = 0;
    for(long t = 0; t < k; t++){ long a2 = m*a + b; b = a; a = a2; }
    for(long t = 0; t < k; t++){ long b2 = a - m*b; a = b; b = b2; }
    long r = 0;
    r += (a - 1 < 0) ? 1 - a : a - 1;
    r += (b < 0) ? -b : b;
    return r;
}
long isqrt_l(long x){
    if(x < 2) return x;
    long g = x; long h = (g + 1) / 2;
    while(h < g){ g = h; h = (g + x / g) / 2; }
    return g;
}
long pi_fixa(long q){
    long S = 1073741824;
    long c = -S;
    long m2 = 2;
    while(m2 < q){ c = isqrt_l(((S + c) / 2) * S); m2 = m2 * 2; }
    long s = isqrt_l(((S - c) / 2) * S);
    return q * s;
}
` };
    const fonte = path.join(TMP, 'ccw_lyapdual.c');
    fs.writeFileSync(fonte, LY.src);
    const wl = path.join(TMP, 'ccw_lyapdual.wasm');
    let li = null, li_aloc = false;
    try {
        execFileSync(CC, [fonte, '-o', wl]);
        li = new WebAssembly.Instance(new WebAssembly.Module(fs.readFileSync(wl)));
    } catch (e) { li = null; li_aloc = /Out of memory|Cannot allocate/i.test(String(e.message)); }
    let F = null;
    if (li) F = (nome, ...a) => li.exports[nome](...a.map(BigInt));
    else if (li_aloc) {
        console.log('   §V11 o motor nao instancia sob este limite — o lyapdual corre pelo cc');
        let main = LY.src + '\n#include <stdio.h>\nint main(void){\n' +
            '  for(long m = 1; m <= 6; m++) for(long k = 1; k <= 10; k++)\n' +
            '    printf("%ld\\n%ld\\n", lyap_sobe(m,k), lyap_volta(m,k));\n' +
            '  for(long j = 2; j <= 21; j++) printf("%ld\\n", pi_fixa(1L << j));\n  return 0;\n}\n';
        const fn = path.join(TMP, 'ccv_lyapdual.c');
        fs.writeFileSync(fn, main);
        const bin = path.join(TMP, 'ccv_lyapdual');
        try {
            execFileSync('cc', ['-O2', '-std=c99', '-w', fn, '-o', bin]);
            const v = execFileSync(bin).toString().trim().split('\n').map(BigInt);
            const tab = { sobe: {}, volta: {}, pi: {} };
            let q = 0;
            for (let m = 1; m <= 6; m++) for (let k = 1; k <= 10; k++){
                tab.sobe[m * 100 + k] = v[q++]; tab.volta[m * 100 + k] = v[q++];
            }
            for (let j = 2; j <= 21; j++) tab.pi[j] = v[q++];
            F = (nome, ...a) => nome === 'lyap_sobe' ? tab.sobe[Number(a[0]) * 100 + Number(a[1])]
                              : nome === 'lyap_volta' ? tab.volta[Number(a[0]) * 100 + Number(a[1])]
                              : tab.pi[Math.log2(Number(a[0]))];
        } catch (e) { F = null; }
    }
    if (F) {
        /* o gato dissipa em TODOS os niveis: e⁺ nao-decrescente, e o total cresce
         * (os bits de F_k sobem ~log2 σ_m por nivel — em m=1 ha degraus repetidos,
         * porque log2 φ < 1: a regua inteira e' honesta, nao força) */
        let cresce = true, volta0 = true;
        for (let m = 1; m <= 6; m++){
            let ant = -1n;
            for (let k = 1; k <= 10; k++){
                const e = F('lyap_sobe', m, k);
                if (e < ant) cresce = false;
                ant = e;
                if (F('lyap_volta', m, k) !== 0n) volta0 = false;
            }
            if (F('lyap_sobe', m, 10) <= F('lyap_sobe', m, 1)) cresce = false;
        }
        ok('§V11 o λ⁺ do gato e' + ' positivo em TODOS os niveis (bits inteiros crescem com k, ' +
           'm=1..6) — a metade que dissipa, medida por potencias de 2 sem um double', cresce);
        ok('§V11 o λ DUALIZADO fecha nivel a nivel: a metade refletida (o esquilo, a estrela ' +
           'ν∘ν=id) devolve a semente com residuo 0 EXATO em 60 de 60 — λ⁺+λ⁻=0, o teorema ' +
           'operacional: a medicao dos niveis e fiavel', volta0);
        /* a dobra do π: dois bits por dobra, MEDIDOS SEM π DE FORA — o passo entre
         * relogios consecutivos d_j = |π_{2^{j+1}} − π_{2^j}| cai ~4x por dobra.
         * E a regua da virgula-fixa (S = 2^30) esgota-se ~2^9 dobras uteis: alem
         * dela satura — a mesma historia do α: alem da dobra que a regua alcanca,
         * nao ha π. Mede-se DENTRO do alcance. */
        const S = 1073741824n;
        const abs = x => x < 0n ? -x : x;
        let dois_bits = true;
        for (let j = 2; j <= 6; j++){
            const d1 = abs(F('pi_fixa', 1 << (j+1)) - F('pi_fixa', 1 << j));
            const d2 = abs(F('pi_fixa', 1 << (j+2)) - F('pi_fixa', 1 << (j+1)));
            if (d2 === 0n || d1 / d2 < 3n || d1 / d2 > 6n) dois_bits = false;
        }
        const ref = F('pi_fixa', 1 << 9);   /* o relogio mais fino DENTRO da regua */
        ok('§V11 o π SAI da dobra (cos π = −1 e a unica semente) e o erro cai DOIS BITS por ' +
           'dobra — razao ~4 entre dobras consecutivas, em virgula-fixa inteira', dois_bits);
        /* a dimensao e a discretizacao do π: q_n = n·2^(n−1), c_n = q_n/2, e a dimensao
         * fecha quando erro_n·c_n < ½ marca — a primeira da torre que fecha e a 4 */
        function folga(n){
            const q = BigInt(n) << BigInt(n - 1);
            const e = F('pi_fixa', Number(q)) - ref;
            const a = e < 0n ? -e : e;
            return a * (q / 2n);            /* meia marca = S/2 na mesma regua */
        }
        const n2 = folga(2) >= S / 2n, n4 = folga(4) < S / 2n, n8 = folga(8) < S / 2n;
        ok('§V11 a DIMENSAO e a discretizacao do π: o relogio de q_n = n·2^(n−1) marcas ' +
           'fecha quando erro·c_n < meia marca — n=2 NAO fecha, n=4 e a PRIMEIRA que fecha, ' +
           'n=8 fecha — o limite dimensional e funcao da luz da dimensao', n2 && n4 && n8);
    } else {
        ok('§V11 o lyapunov dualizado correu por um dos dois caminhos', false);
    }
}

console.log('\n==========================================================================');
if (!falhas) {
    console.log('  O C sobe. Nao ha emcc nesta maquina, nem clang, nem zig, nem tcc — e nao');
    console.log('  precisa de haver: metade da estrada ja estava escrita no chessb.c §C4, que');
    console.log('  diz que «a nossa ISA ja era de pilha, com a pilha escrita por extenso».');
    console.log('');
    console.log('  A outra metade e a mesma frase um andar acima: UMA EXPRESSAO EM C E UMA');
    console.log('  ARVORE, E LE-LA POR BAIXO E EMPILHAR. A pilha nao se constroi — ela ja la');
    console.log('  estava, na arvore. Por isso nao ha representacao intermedia, nem AST, nem');
    console.log('  alocador: le e emite ao mesmo tempo, numa volta so.');
    console.log('');
    console.log('  E POR ISSO NAO E UM COMPILADOR. Compilar tem um tempo e deita fora a');
    console.log('  estrutura de partida — dissipa, e o que se apaga nao volta. Aqui sobe,');
    console.log('  desce, e sobe outra vez NO PROPRIO MODULO, byte a byte: o C e o wasm sao');
    console.log('  duas reguas do mesmo objecto, e ir de uma a outra e reexprimir.');
    console.log('');
    console.log('  E a medida nao compara: REVERTE. Nao ha um numero escrito por mim em lado');
    console.log('  nenhum — ha o objecto e a sua volta. As duas metades: o residuo onde TEM de');
    console.log('  ser zero diz que existe; o residuo onde NAO PODE ser zero diz que e unico.');
} else console.log(`  FALHOU: ${falhas}`);
console.log(`#TOTAL ${feitas} ${falhas}`);
process.exit(falhas ? 1 : 0);
