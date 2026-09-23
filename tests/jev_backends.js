/* jev_backends.js — JEV no motor wasm (piloto E1–E5).
 * E1: Σ_x G(x) = |I|  — o campo de contagem de π conserva o total de eventos.
 * E2: marginais da conjunta = marginais diretas (teorema das marginais do jev.tex),
 *     e cada projecao coincide com o oraculo em JS (sementes deterministicas).
 * E3: tres leituras, uma realizacao — Noul/Choice/Score sobre o mesmo G,
 *     cada leitura uma marginal da leitura conjunta (composicao de E1+E2).
 * E4: Score fracionario como par racional exato (N, |I|) — N = Σ s_i G_q(s_i),
 *     sem divisao no motor; o racional N/|I| é lido do par no oraculo.
 * E5: Paralelismo — Noul e Score no mesmo percurso (duas); G nao e recriado,
 *     e a leitura conjunta coincide com as leituras isoladas (marginal/escore).
 * Realizacoes: LCG fixo (a=1664525, c=1013904223).
 *
 *   node tests/jev_backends.js
 */
import fs from 'node:fs'
import path from 'node:path'
import { execFileSync } from 'node:child_process'
import { fileURLToPath } from 'node:url'

const RAIZ = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..')
const BASE = 8
const SRC = path.join(RAIZ, 'conecthus', 'backends', 'wasm', 'jev', 'marginal.c')
const WASM = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'jev', 'marginal.wasm')

let falhas = 0, feitas = 0
function ok(q, c) { feitas++; if (!c) falhas++; console.log(`#UNIT ${c ? 'ok' : 'falha'} ${q}`) }

console.log('=== JEV no motor wasm (E1+E2) ===\n')

function sobe() {
    fs.mkdirSync(path.dirname(WASM), { recursive: true })
    const precisa = !fs.existsSync(WASM) || fs.statSync(SRC).mtimeMs > fs.statSync(WASM).mtimeMs
    if (!precisa) return
    const trad = path.join(RAIZ, 'tools', 'bin', process.platform === 'win32' ? 'traduz.exe' : 'traduz')
    if (!fs.existsSync(trad)) {
        execFileSync(process.env.CC || 'gcc', ['-O2', '-std=c99', '-w', path.join(RAIZ, 'tools', 'traduz.c'), '-o', trad], { stdio: 'inherit' })
    }
    execFileSync(trad, [SRC, '-o', WASM], { stdio: 'inherit' })
}
sobe()

async function load(nome) {
    const { instance } = await WebAssembly.instantiate(fs.readFileSync(path.join(RAIZ, 'assets', 'figuras', 'wasm', 'jev', nome)))
    return instance.exports
}

function lcg(s) { let x = s >>> 0; return () => ((x = (Math.imul(x, 1664525) + 1013904223) >>> 0) / 4294967296) }
const cla = (b, c, x) => { let i; for (i = 0; i < c; i++) { if (x >= b[i] && x < b[i + 1]) return i } return -1 }
const bounds = (X, c) => { const b = []; for (let j = 0; j <= c; j++) b.push(Math.floor(j * X / c)); return b }
const eq = (a, b, n) => { for (let i = 0; i < n; i++) if (a[i] !== b[i]) return false; return true }
const oracleE4 = (X, G, m, bs, s) => { let N = 0; for (let x = 0; x < X; x++) N += G[x] * s[cla(bs, m, x)]; return N }

function oracle(X, G, cs, bs) {
    const PROD = cs[0] * cs[1] * cs[2]
    const joint = new Array(PROD).fill(0)
    const D1 = [[], [], []].map(() => new Array(16).fill(0))
    const D2 = [[], [], []].map(() => new Array(16).fill(0))
    for (let x = 0; x < X; x++) {
        const g = G[x], y = [cla(bs[0], cs[0], x), cla(bs[1], cs[1], x), cla(bs[2], cs[2], x)]
        joint[y[2] + cs[2] * (y[1] + cs[1] * y[0])] += g
        D1[0][y[0]] += g; D1[1][y[1]] += g; D1[2][y[2]] += g
    }
    let k0 = 0, k1 = 0, k2 = 0
    for (let idx = 0; idx < PROD; idx++) {
        const v = joint[idx]
        D2[0][k0] += v; D2[1][k1] += v; D2[2][k2] += v
        k2++; if (k2 === cs[2]) { k2 = 0; k1++; if (k1 === cs[1]) { k1 = 0; k0++ } }
    }
    return { joint, D1, D2 }
}

const casos = [
    { X: 16, I: 64,   cs: [2, 4, 5] },
    { X: 16, I: 31,   cs: [2, 4, 1] },
    { X: 8,  I: 300,  cs: [5, 2, 4] },
    { X: 64, I: 4444, cs: [3, 3, 3] },
    { X: 32, I: 0,    cs: [2, 2, 1] },
    { X: 32, I: 1,    cs: [4, 2, 2] },
]
const sementes = [111, 222, 333, 777, 999]

;(async () => {
    const ex = await load('marginal.wasm')
    const a = new Int32Array(ex.DISCO.buffer, BASE)

    for (const caso of casos) {
        const { X, I, cs } = caso
        const bs = cs.map(c => bounds(X, c))
        const reais = cs.map((c, i) => c > 1)
        const nReal = reais.filter(Boolean).length
        const c0 = cs[0], c1 = cs[1], c2 = cs[2]
        const B = X + 5, S = c0 + c1 + c2 + 3, D1 = B + S, PROD = c0 * c1 * c2, J = D1 + 48, D2 = J + PROD, OUT = D2 + 48
        for (const s of sementes) {
            const rnd = lcg(s)
            const G = new Array(X).fill(0)
            for (let t = 0; t < I; t++) { const x = Math.floor(rnd() * X); G[x]++ }
            const o = oracle(X, G, cs, bs)
            for (let x = 0; x < X; x++) a[x] = G[x]
            a[X] = I; a[X + 1] = nReal
            a[X + 2] = c0; a[X + 3] = c1; a[X + 4] = c2
            let off = X + 5
            for (let i = 0; i < 3; i++) for (let j = 0; j <= cs[i]; j++) a[off++] = bs[i][j]
            ex.marginal(X)

            const id = `X${X} I${I} c${cs.join('.')} s${s}`

            /* E1: conservacao do campo de contagem */
            ok(`E1 ${id}: contagem total=${a[OUT]} == |I|=${I}`, a[OUT] === I && a[OUT + 1] === I)
            for (let p = 0; p < 3; p++) {
                if (!reais[p]) continue
                let soma = 0
                for (let i = 0; i < cs[p]; i++) soma += a[D1 + 16 * p + i]
                ok(`E1 ${id}: Σ_y D1[${p}]=${soma} == ${I}`, soma === I)
            }

            /* E2: diretas batem com o oraculo e com as marginais da conjunta */
            for (let p = 0; p < 3; p++) {
                if (!reais[p]) continue
                const wD1 = [], wD2 = []
                for (let i = 0; i < 16; i++) { wD1.push(a[D1 + 16 * p + i]); wD2.push(a[D2 + 16 * p + i]) }
                ok(`E2 ${id}: D1[${p}] == oraculo`, eq(wD1, o.D1[p], 16))
                ok(`E2 ${id}: D2[${p}] == oraculo`, eq(wD2, o.D2[p], 16))
                ok(`E2 ${id}: D1[${p}] == D2[${p}] (teorema das marginais)`, eq(wD1, wD2, 16))
            }
        }
    }

    /* §E3 — tres leituras, uma realizacao: Noul/Choice/Score sobre o mesmo G,
     * cada leitura uma marginal da leitura conjunta (compoe E1+E2). */
    {
        const noulB = X => [0, Math.floor(X / 2), X]
        const choiceB = X => bounds(X, 4)
        const scoreB = X => bounds(X, 5)
        const leituras = ['Noul', 'Choice', 'Score']
        const casosE3 = [
            { X: 16, I: 64,   s: [111, 222, 333, 777, 999] },
            { X: 8,  I: 300,  s: [111, 333] },
            { X: 64, I: 4444, s: [111] },
        ]
        for (const c3 of casosE3) {
            const { X, I } = c3
            const cs = [2, 4, 5]
            const bs = [noulB(X), choiceB(X), scoreB(X)]
            const c0 = 2, c1 = 4, c2 = 5
            const B = X + 5, S = c0 + c1 + c2 + 3, D1 = B + S, PROD = c0 * c1 * c2, J = D1 + 48, D2 = J + PROD, OUT = D2 + 48
            for (const s of c3.s) {
                const rnd = lcg(s)
                const G = new Array(X).fill(0)
                for (let t = 0; t < I; t++) { G[Math.floor(rnd() * X)]++ }
                const o = oracle(X, G, cs, bs)
                for (let x = 0; x < X; x++) a[x] = G[x]
                a[X] = I; a[X + 1] = 3
                a[X + 2] = c0; a[X + 3] = c1; a[X + 4] = c2
                let off = X + 5
                for (let i = 0; i < 3; i++) for (let j = 0; j <= cs[i]; j++) a[off++] = bs[i][j]
                ex.marginal(X)
                const id = `X${X} I${I} s${s}`
                const rows = []
                for (let p = 0; p < 3; p++) {
                    const r = []
                    for (let i = 0; i < 16; i++) r.push(a[D1 + 16 * p + i])
                    rows.push(r)
                }
                /* cada leitura normaliza (Σ p_q = 1) e é marginal da conjunta */
                for (let p = 0; p < 3; p++) {
                    let soma = 0
                    for (let i = 0; i < cs[p]; i++) soma += rows[p][i]
                    ok(`E3 ${id} ${leituras[p]}: Σ p_q = ${soma}/${I}`, soma === I)
                    const wD2 = []
                    for (let i = 0; i < 16; i++) wD2.push(a[D2 + 16 * p + i])
                    ok(`E3 ${id} ${leituras[p]}: marginal da conjunta`, eq(rows[p], o.D1[p], 16) && eq(rows[p], wD2, 16))
                }
                /* Choice: distribuição + ponto decisor (argmax c⋆) */
                const argmax = r => { let m = 0; for (let i = 1; i < r.length; i++) if (r[i] > r[m]) m = i; return m }
                const cw = argmax(rows[1]), co = argmax(o.D1[1])
                ok(`E3 ${id} Choice: c⋆=${cw}`, cw === co && rows[1][cw] === o.D1[1][co])
                /* uma realização → leituras distintas */
                ok(`E3 ${id}: Noul≠Score (tres leituras do mesmo G)`, !eq(rows[0], rows[2], 4))
                ok(`E3 ${id}: mesmo G (sem re-criação)`, a[OUT] === I && a[OUT + 1] === I)
            }
        }
    }

    /* §E4 — Score fracionario como par racional exato (N, |I|): o funcional
     * L(p_q)=Σ s_i p_q(s_i) nao precisa de divisao no motor — N=Σ s_i G_q(s_i)
     * sai inteiro; o racional N/|I| é leitura do par, feita no oraculo. */
    {
        const m = 5, s = [0, 1, 2, 3, 4]
        const bs = [0, 1, 3, 5, 7, 9]   /* niveis por celula de X=9 */
        const explicitos = [
            { X: 9, I: 3, cs: { 1: 1, 5: 1, 6: 1 }, esp: '7/3', frac: true },
            { X: 9, I: 4, cs: { 2: 1, 3: 1, 7: 1, 8: 1 }, esp: '11/4', frac: true },
            { X: 9, I: 5, cs: { 0: 5 }, esp: '0/5' },
            { X: 9, I: 6, cs: { 0: 1, 1: 2, 4: 2, 8: 1 }, esp: '5/3', frac: true },
        ]
        for (const caso of explicitos) {
            const { X, I, cs, esp } = caso
            const G = new Array(X).fill(0)
            for (const c of Object.keys(cs)) G[c] = cs[c]
            for (let x = 0; x < X; x++) a[x] = G[x]
            a[X] = I; a[X + 1] = m
            let off = X + 2
            for (let j = 0; j <= m; j++) a[off++] = bs[j]
            for (let j = 0; j < m; j++) a[off++] = s[j]
            ex.escore(X)
            const OUT = off, N = a[OUT], D = a[OUT + 1]
            const Noc = oracleE4(X, G, m, bs, s)
            const val = N / D
            ok(`E4 ${esp}: N_wasm=${N} == N_oraculo=${Noc}`, N === Noc)
            ok(`E4 ${esp}: D_wasm=${D} == |I|=${I}`, D === I)
            ok(`E4 ${esp}: racional ${N}/${D}=${val.toFixed(4)} == E_G[S]`, N === Noc && D === I && val === Noc / I)
            if (caso.frac) ok(`E4 ${esp}: fracionario (nao inteiro)`, N % D !== 0)
        }
        /* robustez: LCG sobre o mesmo escore, pares exatos */
        for (const { X, I } of [{ X: 16, I: 64 }, { X: 8, I: 300 }, { X: 64, I: 4444 }]) {
            for (const seed of [111, 333, 777]) {
                const rnd = lcg(seed)
                const G = new Array(X).fill(0)
                for (let t = 0; t < I; t++) { G[Math.floor(rnd() * X)]++ }
                const bsX = bounds(X, m)
                for (let x = 0; x < X; x++) a[x] = G[x]
                a[X] = I; a[X + 1] = m
                let off = X + 2
                for (let j = 0; j <= m; j++) a[off++] = bsX[j]
                for (let j = 0; j < m; j++) a[off++] = s[j]
                ex.escore(X)
                const OUT = off, N = a[OUT], D = a[OUT + 1]
                const Noc = oracleE4(X, G, m, bsX, s)
                ok(`E4 X${X} I${I} s${seed}: (N=${N},D=${D}) exato`,
                    N === Noc && D === I && N * I === Noc * D)
            }
        }
    }

    /* §E5 — Paralelismo: um disco, duas roupas. Noul e Score no mesmo percurso
     * (duas) sobre o mesmo G; a segunda leitura NAO recria o campo, e a leitura
     * conjunta coincide com as isoladas (marginal para o Noul, escore para o
     * par (N,D)) — «avaliadas em paralelo, cada uma em isolamento». */
    {
        const m = 5, s = [0, 1, 2, 3, 4]
        const noulB = X => [0, Math.floor(X / 2), X]
        const scoreB = X => bounds(X, m)
        for (const caso of [{ X: 16, I: 64 }, { X: 8, I: 300 }, { X: 64, I: 4444 }]) {
            const { X, I } = caso
            for (const seed of [111, 222, 777]) {
                const rnd = lcg(seed)
                const G = new Array(X).fill(0)
                for (let t = 0; t < I; t++) { G[Math.floor(rnd() * X)]++ }
                for (let x = 0; x < X; x++) a[x] = G[x]
                a[X] = I; a[X + 1] = m
                const bn = noulB(X), bs = scoreB(X)
                let off = X + 2
                for (let j = 0; j < 3; j++) a[off++] = bn[j]
                for (let j = 0; j <= m; j++) a[off++] = bs[j]
                for (let j = 0; j < m; j++) a[off++] = s[j]
                const OUT = off
                ex.duas(X)
                const id = `X${X} I${I} s${seed}`
                const n0 = a[OUT], n1 = a[OUT + 1]
                const sc = [], tot = a[OUT + m + 4]
                let somS = 0
                for (let i = 0; i < m; i++) { sc.push(a[OUT + 2 + i]); somS += sc[i] }
                const N = a[OUT + m + 2], D = a[OUT + m + 3]

                /* oraculo das duas leituras */
                const Noc = oracleE4(X, G, m, bs, s)
                const ons = []
                for (let x = 0; x < X; x++) { const y = cla(bn, 2, x); while (ons.length <= y) ons.push(0); ons[y] += G[x] }

                ok(`E5 ${id}: uma varredura, total=${tot} == |I|=${I}`, tot === I)
                ok(`E5 ${id}: Noul conserva (${n0}+${n1})`, n0 + n1 === tot && n0 === ons[0] && n1 === ons[1])
                ok(`E5 ${id}: Score conserva (Σ=${somS})`, somS === tot)
                ok(`E5 ${id}: Noul batem oraculo`, n0 === ons[0] && n1 === ons[1])

                /* G intacto: a segunda leitura nao recriou o campo */
                let gix = true
                for (let x = 0; x < X; x++) if (a[x] !== G[x]) gix = false
                ok(`E5 ${id}: campo G intacto (nao recriado)`, gix && a[X] === I)

                /* leitura conjunta (duas) == leituras isoladas (marginal/escore) */
                a[X + 1] = 1; a[X + 2] = 2; a[X + 3] = 1; a[X + 4] = 1
                let o2 = X + 5
                for (let j = 0; j < 3; j++) a[o2++] = bn[j]
                for (let j = 0; j < 2; j++) a[o2++] = [0, X][j]
                for (let j = 0; j < 2; j++) a[o2++] = [0, X][j]
                ex.marginal(X)
                const D1 = X + 5 + 2 + 1 + 1 + 3
                const iso0 = a[D1], iso1 = a[D1 + 1]

                a[X + 1] = m
                let o3 = X + 2
                for (let j = 0; j <= m; j++) a[o3++] = bs[j]
                for (let j = 0; j < m; j++) a[o3++] = s[j]
                ex.escore(X)
                const OUTo = o3, no = a[OUTo], do_ = a[OUTo + 1]

                ok(`E5 ${id}: Noul conjunta == Noul isolada (${iso0},${iso1})`, n0 === iso0 && n1 === iso1)
                ok(`E5 ${id}: (N,D) conjunta == (N,D) isolada (${N},${D})`, N === Noc && N === no && D === I && D === do_)
            }
        }
    }

    console.log(`#TOTAL ${feitas} ${falhas}`)
    process.exit(falhas ? 1 : 0)
})().catch(e => {
    console.error(e)
    console.log('#TOTAL', feitas, falhas + 1)
    process.exit(1)
})