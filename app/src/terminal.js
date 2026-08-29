// terminal.js — terminal: absorção node_move + canal (fio a fio).

import { initBancoSql, shellPadrao, SHELLS } from './banco_sql.js'
import { abrirCanal } from './canal_browser.js'
import { aresta, traduz, carregaInstanciasPipe } from './banco_tradutor.js'
import { traduzCadeia, paridadeWasmMetal } from './c_wasm_shell.js'
import { execQueryCelula } from './banco_celula.js'
import { loadWasmCelula, validaCelula } from './celula_wasm.js'

export async function initTerminal () {
  await initBancoSql()
  const backend = shellPadrao()
  const sh = SHELLS[backend]
  const sec = document.createElement('section')
  sec.className = 'blk'
  sec.id = 'terminal'
  sec.innerHTML = `
    <div class="wrap">
      <h2>Terminal <span class="sub">· ${sh.label} · absorção</span></h2>
      <div class="rule"></div>
      <p class="term-lede"><code>${backend}_move(−1)</code> ou <code>isa+erg.fita</code> na arena
        · canal sincroniza slots <code>S_CANAL+${sh.slotIn - 9895936}</code></p>
      <div class="term">
        <div class="term-meta">
          <span class="term-estado" data-estado>…</span>
          <span class="term-backend" data-backend>${sh.label}</span>
          <span class="term-banda" data-banda>banda=—</span>
          <span class="term-slot" data-fita>fita=…</span>
        </div>
        <pre class="term-out" data-out aria-live="polite"></pre>
        <form class="term-form" data-form>
          <span class="term-prompt">$</span>
          <input class="term-in" data-in type="text" autocomplete="off" spellcheck="false"
            placeholder="console.log(1)" />
        </form>
        <p class="term-hint"><code>traduz de para texto</code> · auto=fita local · <code>#motor=wasm</code> arena · <code>#remoto</code> Patria.</p>
      </div>
    </div>`
  const outEl = sec.querySelector('[data-out]')
  const inEl = sec.querySelector('[data-in]')
  const form = sec.querySelector('[data-form]')
  const estadoEl = sec.querySelector('[data-estado]')
  const bandaEl = sec.querySelector('[data-banda]')
  const fitaEl = sec.querySelector('[data-fita]')
  let hist = []
  let histPos = -1
  let busy = false
  let canal = null
  let instanciasTraduz = null
  const ctx = {}
  try {
    const hash = location.hash
    const h = hash.match(/motor=(\w+)/)
    if (h) ctx.motor = h[1]
    if (/#remoto\b/.test(hash)) ctx.remoto = true
  } catch (_) { /* */ }

  function linha (text, cls) {
    const span = document.createElement('span')
    span.className = 'term-line' + (cls ? ' ' + cls : '')
    span.textContent = text
    outEl.appendChild(span)
    outEl.appendChild(document.createTextNode('\n'))
    outEl.scrollTop = outEl.scrollHeight
  }

  async function arranque () {
    let arenaOk = false
    try {
      await execQueryCelula("INSERT TEXTO 'ok'")
      arenaOk = true
    } catch (e) { /* */ }
    try {
      const { celula } = await loadWasmCelula(backend, '/wasm/')
      const v = validaCelula(backend, celula)
      if (v.ok) {
        fitaEl.textContent = 'fita=' + v.fitaLen + 'B'
        ctx.temFita = true
        linha('erg.fita ' + v.fitaLen + 'B · motor ' + (ctx.motor || 'auto') +
          (ctx.remoto ? '+remoto' : ''), 'sys')
      } else {
        fitaEl.textContent = 'fita=—'
        linha('wasm sem ' + v.falta + ' — corra gera_nucleo', 'dim')
      }
    } catch (e) {
      fitaEl.textContent = 'fita=—'
      linha(String(e.message || e), 'dim')
    }
    try {
      canal = abrirCanal()
      ctx.canal = canal
      await canal.grava(sh.slotIn, 0, 0)
      const hex = canal.bandaHex()
      if (hex) bandaEl.textContent = 'banda=' + hex
      estadoEl.textContent = arenaOk ? 'arena+canal' : 'canal'
      estadoEl.className = 'term-estado ok'
      linha('canal · slots ' + sh.slotIn + '/' + sh.slotOut +
        (ctx.remoto ? ' · remoto' : ' · local'), 'sys')
    } catch (e) {
      canal = null
      ctx.canal = null
      estadoEl.textContent = arenaOk ? 'arena' : 'offline'
      estadoEl.className = arenaOk ? 'term-estado ok' : 'term-estado mau'
      linha(arenaOk ? 'arena wasm — sem canal' : 'sem arena', 'sys')
    }
  }

  async function run (cmd) {
    const c = cmd.trim()
    if (!c || busy) return
    busy = true
    inEl.disabled = true
    linha('$ ' + c, 'cmd')
    try {
      const m = c.match(/^traduz\s+(\S+)\s+(\S+)\s+(.+)$/s)
      if (m) {
        if (!instanciasTraduz) instanciasTraduz = await carregaInstanciasPipe('/wasm/')
        const edge = aresta(m[1], m[2])
        const { texto: out, rota, par } = await traduz({
          de: m[1], para: m[2], texto: m[3], ctx: { ...ctx, instancias: instanciasTraduz },
        })
        linha('rota ' + rota.join(' → ') + ' · Q' + par.a + par.b, 'sys')
        linha(out, 'out')
        return
      }
      const Q = paridadeWasmMetal(backend)
      const { texto: raw, meta, cadeia, par } = await traduzCadeia(backend, c, ctx)
      const extra = meta?.motor === 'fita'
        ? ' · fita ' + (meta.fitaLen || 0) + 'B · ' + (meta.passos || 0) + ' passos'
        : ''
      linha((meta?.move || backend + '_move') + ' · ' + (meta?.via || 'arena') +
        ' · ' + (meta?.motor || 'wasm') + extra +
        ' · Q' + (par?.a ?? Q.a) + (par?.b ?? Q.b) +
        (cadeia ? ' · ' + cadeia.join('→') : ''), 'sys')
      if (raw) linha(raw, 'out')
      else if (meta?.via === 'arena') linha('(arena — canal para pleno)', 'dim')
      else linha('(sem saída)', 'dim')
    } catch (e) {
      linha(String(e.message || e), 'err')
    } finally {
      busy = false
      inEl.disabled = false
      inEl.focus()
    }
  }

  form.addEventListener('submit', (e) => {
    e.preventDefault()
    const v = inEl.value
    if (!v.trim()) return
    hist.push(v)
    histPos = hist.length
    inEl.value = ''
    run(v)
  })

  inEl.addEventListener('keydown', (e) => {
    if (e.key === 'ArrowUp' && histPos > 0) {
      e.preventDefault()
      histPos--
      inEl.value = hist[histPos]
    } else if (e.key === 'ArrowDown' && histPos < hist.length - 1) {
      e.preventDefault()
      histPos++
      inEl.value = hist[histPos]
    } else if (e.key === 'ArrowDown' && histPos === hist.length - 1) {
      e.preventDefault()
      histPos = hist.length
      inEl.value = ''
    }
  })

  linha('terminal — absorção fio a fio', 'sys')
  arranque()

  return sec
}
