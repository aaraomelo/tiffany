// assistente.js — UI: antena TFAL + ciclo Maestro + rede dual (P↔ℋ↔H).
// Massa → corte → Π → Rede dual (controlo) → Maestro → Y → Metrónomo.
// papers/corpo_peano.tex §rede-dual · §metronomo-ponte

import corpoJson from './corpo.json'
import {
  abrirAntena,
  assinaturaDoCorpo,
  FALA_RESPOSTA,
  FALA_NAO_SEI,
  FALA_ERR,
} from './fala_protocolo.js'
import { abrirDoc, DOCS } from './tex_tradutor.js'
import {
  BATUTA,
  partituraFala,
  massaDeTurno,
  maestroProject,
} from './maestro.js'
import {
  criarEstadoRede,
  passoRedeDual,
  aprendeRedeDual,
} from './rede_dual.js'

/** basename / path → id do tradutor (docs_tradutor.json). */
function mapaTex () {
  const m = new Map()
  for (const [id, path] of Object.entries(DOCS)) {
    const p = String(path).toLowerCase()
    m.set(p, id)
    m.set(p.replace(/^papers\//, ''), id)
    const base = p.split('/').pop()
    m.set(base, id)
    m.set(id.toLowerCase() + '.tex', id)
    m.set(id.replace(/-/g, '_') + '.tex', id)
    m.set(id.replace(/_/g, '-') + '.tex', id)
  }
  return m
}
const TEX_MAP = mapaTex()
const RE_CITA = /(?:papers\/)?[A-Za-z0-9_.-]+\.tex|\/docs\/[A-Za-z0-9_-]+\.pdf/g

function idDaCita (raw) {
  if (raw.endsWith('.pdf')) {
    const m = /\/docs\/([A-Za-z0-9_-]+)\.pdf/.exec(raw)
    return m && DOCS[m[1]] ? m[1] : null
  }
  return TEX_MAP.get(raw.toLowerCase()) || null
}

function primeiraCitaId (texto) {
  RE_CITA.lastIndex = 0
  let m
  while ((m = RE_CITA.exec(texto || ''))) {
    const id = idDaCita(m[0])
    if (id) return id
  }
  return null
}

export function initAssistente () {
  const sec = document.createElement('section')
  sec.className = 'blk'
  sec.id = 'assistente'
  sec.innerHTML = `
    <div class="wrap">
      <h2>A assistente <span class="sub">· P ↔ ℋ ↔ H · Maestro · Metrónomo</span></h2>
      <div class="rule"></div>
      <p class="asst-lede">Rede dual na interface (<code>papers/corpo_peano.tex</code> §rede-dual):
        estaca <code>W=−I</code> (P) · banda de histerese (ℋ) · memória Hopfield (H).
        Massa → corte → <code>Π</code> → controlo dual → cristal → Metrónomo atesta
        <code>λ⁺+λ⁻</code>. Cita <code>.tex</code> → tradutor WASM.</p>
      <div class="asst">
        <div class="asst-meta">
          <span class="asst-estado" data-est>desligada</span>
          <span class="asst-banda" data-banda></span>
          <span class="asst-tick" data-tick title="tick do Maestro">tick 0</span>
          <span class="asst-batuta" data-batuta title="batuta I">I=+1</span>
          <span class="asst-pi" data-pi title="partitura Π (o quê)">Π=—</span>
          <span class="asst-rede" data-rede title="rede dual P/ℋ/H">P·ℋ·H</span>
          <span class="asst-lambda" data-lambda title="λ⁺+λ⁻ (assinatura)">λΣ=—</span>
          <span class="asst-residuo" data-residuo title="resíduo do Metrónomo">r=—</span>
        </div>
        <div class="asst-log" data-log role="log" aria-live="polite"></div>
        <form class="asst-form" data-form>
          <input class="asst-in" data-in type="text" autocomplete="off"
                 placeholder="a fala…" aria-label="fala para a assistente" />
          <button class="btn prim" type="submit" data-avancar>I=+1</button>
          <button class="btn ghost" type="button" data-retrair title="retração">I=−1</button>
        </form>
        <p class="asst-hint">Daemon:
          <code>cd banco && CONVERSA=./bin/conversa ./fala</code>
          · ensinar: <code>= resposta</code>
          · <code>r&gt;0</code> + massa faltante → ensinar ou ferramenta
          · rede: ℱ:(x,h)↦(x′,h′); |u|≤Δ retém; Hopfield=memória Y; λ⁻ da conjugação
          · Π=quê · I=sentido · ℱ=ramo · r=atestação (Prop. pi-cadeia)
          · cita <code>.tex</code> → tradutor · §rede-dual / §musica</p>
      </div>
    </div>`

  const log = sec.querySelector('[data-log]')
  const est = sec.querySelector('[data-est]')
  const bandaEl = sec.querySelector('[data-banda]')
  const tickEl = sec.querySelector('[data-tick]')
  const batutaEl = sec.querySelector('[data-batuta]')
  const piEl = sec.querySelector('[data-pi]')
  const residuoEl = sec.querySelector('[data-residuo]')
  const redeEl = sec.querySelector('[data-rede]')
  const lambdaEl = sec.querySelector('[data-lambda]')
  const form = sec.querySelector('[data-form]')
  const input = sec.querySelector('[data-in]')
  const btnRetrair = sec.querySelector('[data-retrair]')

  let antena = null
  let ultimaFala = ''
  let tick = 0
  const historico = []
  const assinatura = assinaturaDoCorpo(corpoJson)
  const rede = criarEstadoRede({ c: 0, delta: 0.18 })

  function mostraRede (passo) {
    if (!passo) {
      redeEl.textContent = 'P·ℋ·H'
      redeEl.className = 'asst-rede'
      lambdaEl.textContent = 'λΣ=—'
      lambdaEl.className = 'asst-lambda'
      return
    }
    const tag = passo.acao === 'reter' ? 'ℋ' : passo.acao === 'volta' ? 'H' : 'P'
    const X = passo.X || { x: passo.x, h: passo.hAnt }
    redeEl.textContent = tag +
      ' X=(' + Number(X.x).toFixed(2) + ',' + X.h + ')' +
      (passo.banda ? ' ·banda' : '')
    redeEl.className = 'asst-rede' +
      (passo.acao === 'reter' ? ' reter' : passo.acao === 'volta' ? ' volta' : ' frente')
    redeEl.title = passo.motivo +
      (passo.Xn ? ' → X′=(' + passo.Xn.x.toFixed(2) + ',' + passo.Xn.h + ')' : '')
    const soma = passo.lambdaSoma
    lambdaEl.textContent = 'λΣ=' + (Number.isFinite(soma) ? soma.toFixed(2) : '—')
    lambdaEl.className = 'asst-lambda' + (Math.abs(soma) < 0.05 ? ' ok' : '')
    lambdaEl.title = 'λ⁺=' + passo.lambdaP.toFixed(2) +
      ' λ⁻=' + passo.lambdaH.toFixed(2) + ' (assinatura; Metrónomo atesta r)'
  }

  function mostraCiclo (batuta, residuo, passoRede, pi) {
    tickEl.textContent = 'tick ' + tick
    const nome = batuta === BATUTA.RETRAIR ? 'I=−1' : batuta === BATUTA.NEUTRO ? 'I=0' : 'I=+1'
    batutaEl.textContent = nome
    if (pi && pi.objetivo) {
      const o = String(pi.objetivo)
      piEl.textContent = 'Π=' + (o.length > 28 ? o.slice(0, 26) + '…' : o)
      piEl.title = 'partitura Π (o quê): ' + o +
        (pi.esquema ? ' · ' + pi.esquema : '')
      piEl.className = 'asst-pi ok'
    } else {
      piEl.textContent = 'Π=—'
      piEl.className = 'asst-pi'
      piEl.title = 'partitura Π (o quê)'
    }
    mostraRede(passoRede)
    if (!residuo) {
      residuoEl.textContent = 'r=—'
      residuoEl.className = 'asst-residuo'
      return
    }
    residuoEl.textContent = 'r=' + residuo.r + (residuo.suporteFaltante ? ' · massa+' : '')
    residuoEl.className = 'asst-residuo' + (residuo.ok ? ' ok' : ' mau')
    residuoEl.title = residuo.motivo || ''
  }

  function linkify (el, texto) {
    RE_CITA.lastIndex = 0
    let last = 0
    let m
    let alguma = false
    while ((m = RE_CITA.exec(texto))) {
      if (m.index > last) el.appendChild(document.createTextNode(texto.slice(last, m.index)))
      const raw = m[0]
      const id = idDaCita(raw)
      if (id) {
        alguma = true
        const a = document.createElement('a')
        a.href = `/docs/${id}.pdf`
        a.className = 'asst-tex'
        a.textContent = raw
        a.title = `compor ${id} no tradutor (WASM)`
        a.addEventListener('click', async (ev) => {
          ev.preventDefault()
          const janela = window.open('about:blank', '_blank')
          a.classList.add('busy')
          try {
            await abrirDoc(id, janela)
          } catch (err) {
            if (janela && !janela.closed) janela.close()
            linha('sys', 'tradutor', err.message || String(err), false)
          } finally {
            a.classList.remove('busy')
          }
        })
        el.appendChild(a)
      } else {
        el.appendChild(document.createTextNode(raw))
      }
      last = m.index + raw.length
    }
    if (last < texto.length) el.appendChild(document.createTextNode(texto.slice(last)))
    if (!alguma && last === 0) el.textContent = texto
  }

  function linha (cls, quem, texto, ponte = false) {
    const d = document.createElement('div')
    d.className = 'asst-msg ' + cls
    const q = document.createElement('span')
    q.className = 'quem'
    q.textContent = quem
    const t = document.createElement('span')
    t.className = 'txt'
    if (ponte) linkify(t, texto || '')
    else t.textContent = texto || ''
    d.appendChild(q)
    d.appendChild(t)
    log.appendChild(d)
    log.scrollTop = log.scrollHeight
  }

  async function garanteAntena () {
    if (antena) return antena
    est.textContent = 'a ligar…'
    antena = abrirAntena()
    const hello = await antena.hello(assinatura)
    est.textContent = 'na banda'
    bandaEl.textContent = antena.bandaHex() || ''
    bandaEl.title = assinatura.trim()
    if (hello.texto) linha('sys', 'antena', hello.texto, false)
    return antena
  }

  /** Motor: cristal TFAL — realiza π_k(X), não é o Maestro. */
  async function motorCristal (X, pi) {
    await garanteAntena()
    const fala = pi.objetivo || (X.suporte[X.suporte.length - 1] || '')
    const fr = await antena.fala(fala)
    if (fr.op === FALA_NAO_SEI) {
      return {
        Y: fr.texto || 'não sei',
        op: 'nao_sei',
        naoSei: true,
        doCristal: true,
      }
    }
    if (fr.op === FALA_ERR) {
      return { Y: fr.texto || '', op: 'erro', erro: fr.texto, doCristal: true }
    }
    if (fr.op === FALA_RESPOSTA) {
      return { Y: fr.texto || '', op: 'resposta', doCristal: true }
    }
    return { Y: fr.texto || '', op: 'op' + fr.op, doCristal: true }
  }

  async function ciclo (fala, I) {
    const X = massaDeTurno(fala, historico)
    const pi = partituraFala(fala)

    // Controlo dual: P / ℋ / H (corpo_peano §rede-dual)
    const passo = passoRedeDual(rede, fala)
    linha('sys', 'rede', passo.motivo +
      (passo.recall ? ' · recall=' + passo.recall.fonte : ''), false)

    // Retenção na banda: emite Y guardado sem novo projectar (I=0)
    if (passo.acao === 'reter' && passo.recall && passo.recall.Y && I !== BATUTA.RETRAIR) {
      tick += 1
      const residuo = {
        r: 0,
        ok: true,
        suporteFaltante: false,
        motivo: 'r=0 — retenção ℋ (h′=h); Metrónomo: volta trivial na banda',
      }
      aprendeRedeDual(rede, fala, passo.recall.Y, passo)
      mostraCiclo(BATUTA.NEUTRO, residuo, passo, pi)
      historico.push(fala, passo.recall.Y)
      linha('ela', 'assistente', passo.recall.Y, true)
      linha('sys', 'metrónomo', residuo.motivo +
        ' · λΣ=' + passo.lambdaSoma.toFixed(2), false)
      return
    }

    // Volta Hopfield: Y da memória associativa; Metrónomo atesta
    if (passo.acao === 'volta' && passo.recall && passo.recall.Y && I === BATUTA.AVANCAR) {
      tick += 1
      const residuo = {
        r: 0,
        ok: true,
        suporteFaltante: false,
        motivo: 'r=0 — volta H (Hopfield); dualidade λ⁺+λ⁻ (assinatura)',
      }
      aprendeRedeDual(rede, fala, passo.recall.Y, {
        ...passo,
        lambdaH: -passo.lambdaP,
        lambdaSoma: 0,
      })
      mostraCiclo(BATUTA.AVANCAR, residuo, {
        ...passo,
        lambdaH: -passo.lambdaP,
        lambdaSoma: 0,
      }, pi)
      historico.push(fala, passo.recall.Y)
      linha('ela', 'assistente', passo.recall.Y, true)
      linha('sys', 'metrónomo', residuo.motivo +
        ' · overlap=' + (passo.recall.overlap != null
          ? passo.recall.overlap.toFixed(2) : '?'), false)
      const id = primeiraCitaId(passo.recall.Y)
      if (id) {
        linha('sys', 'tradutor', 'a compor ' + id + '…', false)
        const janela = window.open('about:blank', '_blank')
        try { await abrirDoc(id, janela) } catch (err) {
          if (janela && !janela.closed) janela.close()
          linha('sys', 'tradutor', err.message || String(err), false)
        }
      }
      return
    }

    const Iefetivo = I === BATUTA.RETRAIR ? BATUTA.RETRAIR : BATUTA.AVANCAR
    const saida = await maestroProject({
      pi,
      X,
      tick,
      I: Iefetivo,
      projectar: motorCristal,
    })
    tick = saida.tick

    if (saida.retraido) {
      mostraCiclo(saida.batuta, saida.residuo, passo, pi)
      linha('sys', 'maestro', 'retração I=−1 — sem emitir; ajusta Π ou ensina (= resposta)', false)
      return
    }

    const r = saida.residuo
    if (r && r.suporteFaltante) {
      mostraCiclo(saida.batuta, r, passo, pi)
      linha('ela nao', 'assistente', saida.Y || 'não sei', false)
      linha('sys', 'metrónomo', r.motivo + ' · suporte=' + X.suporte.length +
        ' borda=' + X.borda.length, false)
      return
    }
    if (r && !r.ok) {
      mostraCiclo(saida.batuta, r, passo, pi)
      linha('sys', 'metrónomo', 'r=' + r.r + ' — ' + r.motivo, false)
      if (saida.Y) linha('ela', 'assistente', saida.Y, true)
      return
    }
    if (saida.Y) {
      // frente P: aprende padrão + h
      const passoOk = {
        ...passo,
        acao: 'frente',
        lambdaH: r && r.ok ? -passo.lambdaP : 0,
        lambdaSoma: r && r.ok ? 0 : passo.lambdaP,
      }
      aprendeRedeDual(rede, fala, saida.Y, passoOk)
      mostraCiclo(saida.batuta, r, passoOk, pi)
      historico.push(fala, saida.Y)
      linha('ela', 'assistente', saida.Y, true)
      if (r && r.motivo) {
        linha('sys', 'metrónomo', r.motivo +
          (r.ok ? ' · λΣ≈0 (fecho)' : ''), false)
      }
      const id = primeiraCitaId(saida.Y)
      if (id && r && r.ok) {
        linha('sys', 'tradutor', 'a compor ' + id + '…', false)
        const janela = window.open('about:blank', '_blank')
        try {
          await abrirDoc(id, janela)
        } catch (err) {
          if (janela && !janela.closed) janela.close()
          linha('sys', 'tradutor', err.message || String(err), false)
        }
      }
    } else {
      mostraCiclo(saida.batuta, r, passo, pi)
    }
  }

  form.addEventListener('submit', async (e) => {
    e.preventDefault()
    const fala = input.value.trim()
    if (!fala) return
    input.value = ''

    if (fala.startsWith('=') && ultimaFala) {
      const resp = fala.slice(1).trim()
      if (!resp) return
      try {
        await garanteAntena()
        await antena.aprende(ultimaFala, resp)
        historico.push(ultimaFala, resp)
        linha('eu', 'tu', '= ' + resp)
        linha('ela', 'assistente', 'aprendido — massa aumentada na banda')
        mostraCiclo(BATUTA.AVANCAR, { r: 0, ok: true, motivo: 'massa+ via APRENDE' },
          null, partituraFala(ultimaFala))
      } catch (err) {
        est.textContent = 'erro'
        linha('sys', 'erro', err.message || String(err))
        antena = null
      }
      return
    }

    ultimaFala = fala
    linha('eu', 'tu', fala)
    try {
      await ciclo(fala, BATUTA.AVANCAR)
    } catch (err) {
      est.textContent = 'erro'
      linha('sys', 'erro', (err.message || String(err)) +
        ' — sobe o daemon: cd banco && CONVERSA=./bin/conversa ./fala')
      antena = null
      mostraCiclo(BATUTA.AVANCAR, { r: 1, ok: false, motivo: String(err.message || err) },
        null, partituraFala(fala))
    }
  })

  btnRetrair.addEventListener('click', async () => {
    const fala = input.value.trim() || ultimaFala || '(retração)'
    try {
      await ciclo(fala, BATUTA.RETRAIR)
    } catch (err) {
      linha('sys', 'erro', err.message || String(err))
    }
  })

  return sec
}
