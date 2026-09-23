// ── O SELO DOS CONTRATOS TIPADOS (E6) — a construção atravessou o front ──
// A fase de painel_motor.wasm (ponto fixo 2^20, wrap bit-a-bit, resíduo 0) É a realização única:
// cada tick do relógio único (relogio.js) é um evento (passos dt), e a fração da volta bina em
// células (bins x_k). As perguntas tipadas — Noul/Choice/Score — leem a fase JÁ PRONTA no topo do
// quadro (somos um consumidor do caminho livre) e o MESMO marginal.wasm que o bloco E1–E5 validou
// (509/509) re-conta o campo G. O oráculo re-computa em JS; o selo diz: WASM ≡ oráculo ≡ esperado.
// Nada de novo aqui: é a construção E1–E5, sem novo mecanismo de contagem — o último selo do piloto.
// Padrão substratos.js: uma seção ao vivo, badges, tabela por pergunta, botão ↻.
import { faseDoMotor, temWasm } from './motor_wasm.js'
import { registra } from './relogio.js'

const BASE = 8                                    // o disco do marginal.wasm começa no byte 8 (o contrato)
const X = 16                                      // as células (bins) da realização
const S = [0, 1, 2, 3, 4]                         // o Score: cinco níveis (os pesos s_i)
const m = S.length
const bounds = (X, c) => { const b = []; for (let j = 0; j <= c; j++) b.push(Math.floor(j * X / c)); return b }
const LEITURAS = [                                // as três perguntas tipadas (as mesmas do medidor §E3)
  { nome: 'Noul', c: 2, fronteiras: [0, Math.floor(X / 2), X] },
  { nome: 'Choice', c: 4, fronteiras: bounds(X, 4) },
  { nome: 'Score', c: 5, fronteiras: bounds(X, 5) },
]
const cla = (b, c, x) => { for (let i = 0; i < c; i++) { if (x >= b[i] && x < b[i + 1]) return i } return c - 1 }
const argmax = (r) => { let b = 0; for (let i = 1; i < r.length; i++) if (r[i] > r[b]) b = i; return b }

const esc = (s) => String(s).replace(/[&<>]/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;' }[c]))
const badge = (txt, ok) => `<span class="sbadge${ok ? ' ok' : (ok === false ? ' err' : '')}"><span class="sdot"></span>${txt}</span>`
const verifica = (v) => v === true ? '<span class="stag pass">ok</span>' : (v === false ? '<span class="stag drop">≢</span>' : '<span class="stag drop">·</span>')

let G = new Array(X).fill(0)                      // o campo de contagem da realização
let eventos = 0                                   // |I| — os ticks já lidos
let ex = null, wasmTentado = false                // o marginal.wasm (o do bloco E1–E5)
let view = null
let ocupado = false, contador = 0

async function carregaMarginal () {
  try {
    const { instance } = await WebAssembly.instantiate(await (await fetch('/wasm/jev/marginal.wasm')).arrayBuffer())
    ex = instance.exports
    view = new Int32Array(ex.DISCO.buffer, BASE)
  } catch (e) { ex = null }
}

// o oráculo — a MESMA construção em JS puro (do medidor E1–E5, sem o wasm)
function oraculo () {
  const out = LEITURAS.map((l) => {
    const gq = new Array(l.c).fill(0)
    for (let x = 0; x < X; x++) gq[cla(l.fronteiras, l.c, x)] += G[x]
    return gq
  })
  let N = 0
  for (let k = 0; k < m; k++) N += S[k] * out[2][k]   // N = Σ s_i G_q(s_i) — inteiro, o numerador exacto
  return { out, N }
}

// o marginal.wasm: projecção directa (D1) e marginal da conjunta (D2) — o teorema por pergunta
function leMarginal () {
  const c0 = 2, c1 = 4, c2 = 5
  const B = X + 5, So = c0 + c1 + c2 + 3, D1 = B + So, PROD = c0 * c1 * c2, J = D1 + 48, D2 = J + PROD, OUT = D2 + 48
  for (let x = 0; x < X; x++) view[x] = G[x]
  view[X] = eventos; view[X + 1] = 3
  view[X + 2] = c0; view[X + 3] = c1; view[X + 4] = c2
  let off = X + 5
  for (let i = 0; i < 3; i++) for (let j = 0; j <= LEITURAS[i].c; j++) view[off++] = LEITURAS[i].fronteiras[j]
  ex.marginal(X)
  return {
    total: view[OUT],
    rows: LEITURAS.map((l, i) => ({
      d1: Array.from({ length: l.c }, (_, k) => view[D1 + 16 * i + k]),
      d2: Array.from({ length: l.c }, (_, k) => view[D2 + 16 * i + k]),
    })),
  }
}

// o escore: o par exacto (N, |I|) — sem divisão no motor; os níveis G_q(s_i)
// vêm da coordenada Score da leitura conjunta (marginal), não daqui (contrato do C)
function leEscore () {
  const bs = LEITURAS[2].fronteiras
  view[X] = eventos; view[X + 1] = m
  let off = X + 2
  for (let j = 0; j <= m; j++) view[off++] = bs[j]
  for (let j = 0; j < m; j++) view[off++] = S[j]
  ex.escore(X)
  const OUT = X + 2 + (m + 1) + m
  return { N: view[OUT], D: view[OUT + 1] }
}

// uma tabela por pergunta: a directa (D1), a conjunta (D2), o oráculo — e os selos
function montaTabela (q, idx, o, mR, eR) {
  const c = q.c
  const oy = o.out[idx]
  const foiDisco = !!mR
  const teo = foiDisco &&
    mR.rows[idx].d1.every((v, k) => v === oy[k]) &&
    mR.rows[idx].d2.every((v, k) => v === oy[k]) &&
    mR.rows[idx].d1.every((v, k) => v === mR.rows[idx].d2[k])
  const cells = (vals) => vals.map((v) => `<td class="c">${v}</td>`).join('')
  const row = (rot, vals, seloOk) =>
    `<tr><td class="snode"><b>${rot}</b></td>${cells(vals)}<td class="c ssum">${eventos}</td><td class="c">${verifica(seloOk)}</td></tr>`
  const th = `<tr><th>${q.nome} · G_q(y)</th>${Array.from({ length: c }, (_, k) => `<th class="c">y${k}</th>`).join('')}<th class="c">Σ</th><th class="c">selo</th></tr>`
  let body = ''
  if (foiDisco) {
    body += row('projecção directa (D1)', mR.rows[idx].d1, teo)
    body += row('marginal da conjunta (D2)', mR.rows[idx].d2, teo)
  }
  body += row('oráculo (JS)', oy, null)
  if (q.nome === 'Score' && foiDisco) {
    const okPar = eR.N === o.N && eR.D === eventos
    body += `<tr class="jeocus"><td colspan="${c + 2}">par exacto (N, |I|) = (${eR.N}, ${eR.D}) · E_G[S] = ${eR.N}/${eR.D} = ${(eR.N / eR.D).toFixed(3)} — a média fracionária, leitura do par sem divisão no motor</td><td class="c">${verifica(okPar)}</td></tr>`
  }
  if (q.nome === 'Choice' && foiDisco) {
    const cw = argmax(oy)
    body += `<tr class="jeocus"><td colspan="${c + 2}">ponto decisor c⋆ = ${cw} (máx ${oy[cw]}) — o argmax da distribuição, nada mais</td><td class="c">${verifica(cw === argmax(mR.rows[idx].d1))}</td></tr>`
  }
  return `<table class="stable"><thead>${th}</thead><tbody>${body}</tbody></table>`
}

// re-sela com o campo actual: wasm ≡ oráculo, por pergunta, e o campo não é recriado
async function selo () {
  if (ocupado || !document.getElementById('jev-tabelas')) return
  ocupado = true
  try {
    if (!ex && !wasmTentado) { wasmTentado = true; await carregaMarginal() }
    const o = oraculo()
    let mR = null, eR = null, okSelo = false, okCampo = false
    if (ex) {
      mR = leMarginal()
      eR = leEscore()
      okSelo = mR.total === eventos &&
        LEITURAS.every((l, i) =>
          mR.rows[i].d1.every((v, k) => v === o.out[i][k]) &&
          mR.rows[i].d2.every((v, k) => v === o.out[i][k]) &&
          mR.rows[i].d1.every((v, k) => v === mR.rows[i].d2[k])) &&
        eR.D === eventos && eR.N === o.N
      okCampo = true
      for (let x = 0; x < X; x++) if (view[x] !== G[x]) okCampo = false
    }
    const se = document.getElementById('jev-eventos')
    if (se) se.textContent = `|I| = ${eventos}`
    const b1 = document.getElementById('b-campo')
    if (b1) b1.innerHTML = badge(temWasm() ? 'a fase (painel_motor.wasm)' : 'a fase (ponto fixo em JS)', eventos > 0 || temWasm())
    const b2 = document.getElementById('b-wasm')
    if (b2) b2.innerHTML = ex ? badge('marginal.wasm do bloco E1–E5', okSelo) : (wasmTentado ? badge('marginal.wasm indisponível', false) : badge('marginal.wasm …', null))
    const b3 = document.getElementById('b-selo')
    if (b3) b3.innerHTML = badge('WASM ≡ oráculo ≡ esperado', okSelo)
    const b4 = document.getElementById('b-campoG')
    if (b4) b4.innerHTML = badge('o campo G não é recriado', okCampo)
    const tb = document.getElementById('jev-tabelas')
    if (tb) {
      tb.innerHTML = LEITURAS.map((l, i) => montaTabela(l, i, o, mR, eR)).join('')
      tb.querySelectorAll('tr').forEach((t) => { t.style.animation = 'none' })
    }
  } finally { ocupado = false }
}

// o consumidor do caminho livre: lê a fase JÁ PRONTA (o relógio avançou no topo do quadro) e realiza
function tick () {
  const f = faseDoMotor()
  if (Number.isFinite(f) && f >= 0) {
    let x = Math.floor(f * X)
    if (x > X - 1) x = X - 1
    G[x]++; eventos++
  }
  if (++contador % 12 === 0) selo()               // a tabela re-sela 5×/s; o campo acumula todo o quadro
}

export function initJevSelo () {
  const sec = document.createElement('section')
  sec.className = 'blk'; sec.id = 'jevselo'
  sec.innerHTML = `
    <div class="wrap">
      <h2>Os contratos tipados <span class="sub">· a construção E1–E5, no relógio do reino</span></h2>
      <div class="rule"></div>
      <p class="sdesc">A <b>fase</b> de <code>painel_motor.wasm</code> (ponto fixo 2<sup>20</sup>, wrap bit-a-bit,
        resíduo 0) <b>é</b> a realização: cada tick do relógio único é um evento, e a fração da volta bina em
        <code>X=${X}</code> células. As perguntas tipadas (Noul/Choice/Score) leem a fase <b>já pronta</b> no topo do
        quadro — e o mesmo <code>marginal.wasm</code> que o bloco E1–E5 validou (509/509) re-conta o campo
        <code>G</code>; o oráculo re-computa em JS, e o selo confirma <b>WASM ≡ oráculo ≡ esperado</b>.</p>
      <div class="console" id="jev-console">
        <div class="sbar">
          <span class="scall"><b>realização</b>: a fase do painel (passos dt = eventos; bins x_k = células) · <code>X=${X}</code> · <b id="jev-eventos">|I| = 0</b></span>
          <span class="sspacer"></span>
          <span id="b-campo"></span>
          <span id="b-wasm"></span>
          <span id="b-selo"></span>
          <span id="b-campoG"></span>
          <button id="jev-run" class="btnrun" type="button">↻ esvazia o campo</button>
        </div>
        <div class="sscroll" id="jev-tabelas"></div>
      </div>
      <p class="snote">Por pergunta, <code>G_q(y)</code> vem por dois caminhos — a <b>projecção directa</b>
        (D1) e a <b>marginal da leitura conjunta</b> (D2): <code>G_q(y) = Σ<sub>demais coordenadas</sub>
        G_q(y, …)</code> — e os dois coincidem <i>e</i> coincidem com o oráculo (o teorema, exato em inteiros).
        O Score devolve o par exacto (N, |I|) com E<sub>G</sub>[S] = N/|I| — a média fracionária como leitura
        do par, sem divisão no motor. E o campo <code>G</code> fica intacto (a leitura não o recria).</p>
    </div>`
  sec.querySelector('#jev-run').addEventListener('click', () => { G.fill(0); eventos = 0; contador = 0; selo() })
  registra(tick)                                   // entra no relógio único — a fase já avança antes de nós
  selo()
  return sec
}