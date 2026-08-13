// As pontes: C, Dafny e Haskell descem ao Chrome pela ponte da CPU (WebAssembly).
// Cada operação do colapso grafo→LaTeX roda em WASM no navegador, atestada contra a linguagem real.
// Módulos do traduz exportam DISCO (base NULO=8); os do chessc legado exportam mem (base 0).

const INP = 1024, OUTP = 8192

async function inst(url) {
  const b = await (await fetch(url)).arrayBuffer()
  return (await WebAssembly.instantiate(b)).instance
}

function disco(exports) {
  const mem = exports.DISCO || exports.mem
  if (!mem) throw new Error('módulo sem DISCO/mem')
  const base = exports.DISCO ? 8 : 0
  return {
    u8: () => new Uint8Array(mem.buffer),
    i32: () => new Int32Array(mem.buffer, base),
    base,
  }
}

export async function runPontes(data) {
  const nos = data.nos, n = nos.length
  const enc = new TextEncoder(), dec = new TextDecoder()

  const ce = await inst(data.linguagens[0].wasm)          // C: escapar
  const escapar = (t) => {
    const d = disco(ce.exports)
    const m = d.u8()
    const b = enc.encode(t); m.set(b, d.base + INP)
    const ln = ce.exports.escapar(INP, b.length, OUTP)
    return dec.decode(m.slice(d.base + OUTP, d.base + OUTP + ln))
  }
  const cd = await inst(data.linguagens[1].wasm)          // Dafny: decidir (batch)
  const dd = disco(cd.exports)
  const i32 = dd.i32()
  for (let i = 0; i < n; i++) { i32[i] = nos[i].prof; i32[n + i] = nos[i].epist }
  cd.exports.decidir(n)
  const dcd = nos.map((_, i) => [i32[2 * n + i], i32[3 * n + i]])

  const ch = await inst(data.linguagens[2].wasm)          // Haskell: rotular
  const rotular = (t) => {
    const d = disco(ch.exports)
    const m = d.u8()
    const b = enc.encode(t); m.set(b, d.base + INP)
    const ln = ch.exports.rotular(INP, b.length, OUTP)
    return dec.decode(m.slice(d.base + OUTP, d.base + OUTP + ln))
  }

  return nos.map((no, i) => {
    const c = escapar(no.titulo)
    const d = [data.cmd[dcd[i][0]], data.est[dcd[i][1]]]
    const h = rotular(no.id)
    return {
      id: no.id, titulo: no.titulo,
      c, c_ok: c === no.escapado,
      d, d_ok: d[0] === no.comando && d[1] === no.estatuto,
      h, h_ok: h === no.rotulo,
    }
  })
}

const esc = (s) => String(s).replace(/[&<>]/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;' }[c]))
const trunc = (s, n) => (s.length > n ? s.slice(0, n - 1) + '…' : s)
const badge = (txt, ok) => `<span class="sbadge${ok ? ' ok' : (ok === false ? ' err' : '')}"><span class="sdot"></span>${esc(txt)}</span>`

export function initPontes(data) {
  const sec = document.createElement('section')
  sec.className = 'blk'; sec.id = 'pontes'
  const L = data.linguagens
  sec.innerHTML = `
    <div class="wrap">
      <h2>As pontes <span class="sub">· cada linguagem desce ao Chrome pela ponte da CPU</span></h2>
      <div class="rule"></div>
      <p class="sdesc">Tudo é fractal, inclusive as linguagens — cada uma serve pra algo. O <b>WebAssembly é a
        ponte universal da CPU</b>: por ela, três corpos do colapso grafo→LaTeX descem ao navegador, cada um no seu
        Φ — <b>C</b> escapa os metacaracteres, <b>Dafny</b> decide (e o Z3 prova), <b>Haskell</b> rotula. Rodam
        <b>agora, nesta página</b>, e cada um bate com a sua linguagem real (o oráculo no metal). <i>A GPU é a mesma
        broca, só no campo</i> — o filtro, acima.</p>
      <div class="console">
        <div class="sbar">
          <span class="scall"><b>3 × WebAssembly.instantiate</b> · C ${L[0].bytes}B · Dafny ${L[1].bytes}B · Haskell ${L[2].bytes}B · ${data.nos.length} nós reais</span>
          <span class="sspacer"></span>
          <span id="p-c">${badge('C …', null)}</span>
          <span id="p-d">${badge('Dafny …', null)}</span>
          <span id="p-h">${badge('Haskell …', null)}</span>
          <button id="p-run" class="btnrun" type="button">↻ rodar de novo</button>
        </div>
        <div class="sscroll"><table class="stable">
          <thead><tr>
            <th>nó (grafo)</th>
            <th>C · escaping LaTeX</th>
            <th>Dafny · comando \\ estatuto</th>
            <th>Haskell · rótulo</th>
          </tr></thead>
          <tbody id="p-rows"></tbody>
        </table></div>
      </div>
      <p class="snote">Cada <code>.wasm</code> foi emitido e <b>atestado no metal</b> por um nó do grafo —
        <code>app_pontes.py</code> — contra a linguagem nativa: o <b>cc</b> (C), o <b>ghc</b> (Haskell) e o
        <b>dafny</b> (Dafny), este com o <b>Z3</b> provando a decisão (<code>dafny verify</code>). O navegador só
        re-encena, na ponte da CPU. As mesmas linguagens seguem no seu Φ nativo — a ponte apenas as traz à Web.</p>
    </div>`
  const rows = sec.querySelector('#p-rows')
  const run = () => {
    runPontes(data).then((res) => {
      const okC = res.every((r) => r.c_ok), okD = res.every((r) => r.d_ok), okH = res.every((r) => r.h_ok)
      sec.querySelector('#p-c').innerHTML = badge(`C ≡ cc`, okC)
      sec.querySelector('#p-d').innerHTML = badge(`Dafny ≡ dafny${data.z3_provou ? ' · Z3 ✓' : ''}`, okD && data.z3_provou)
      sec.querySelector('#p-h').innerHTML = badge(`Haskell ≡ ghc`, okH)
      rows.innerHTML = ''
      res.forEach((r, i) => {
        const tr = document.createElement('tr')
        tr.style.animationDelay = (i * 16) + 'ms'
        const chk = (ok) => ok ? '<span class="sok">✓</span>' : '<span class="serr">✗</span>'
        tr.innerHTML =
          `<td class="snode"><b>${esc(trunc(r.id, 22))}</b></td>` +
          `<td class="scode">${chk(r.c_ok)} ${esc(trunc(r.c, 30))}</td>` +
          `<td class="scode">${chk(r.d_ok)} <span class="srel">${esc(r.d[0])}</span> \\ ${esc(r.d[1])}</td>` +
          `<td class="scode">${chk(r.h_ok)} ${esc(trunc(r.h, 26))}</td>`
        rows.appendChild(tr)
      })
    }).catch((e) => {
      sec.querySelector('#p-c').innerHTML = badge('WASM indisponível', false)
      console.error('pontes:', e)
    })
  }
  sec.querySelector('#p-run').addEventListener('click', run)
  requestAnimationFrame(run)
  return sec
}
