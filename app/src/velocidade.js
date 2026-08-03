// ── O CONTROLE DE VELOCIDADE — o circuito síncrono dos cards (context2d, pixel a pixel) ──
// O slider é ω (a referência do painel). O RELÓGIO ÚNICO (a fase do painel_motor.wasm, faseDoMotor) dirige
// TODOS os cards: cada um completa 1 CICLO por 2π de fase — a mesma fase do coração (cos). Assim o wrap
// (2π→0) é CONTÍNUO para todos (idx n→0, o crossfade emenda), sem a descontinuidade dos períodos
// incomensuráveis. Os GIFs são decodificados (ImageDecoder, lazy — só os visíveis, em paralelo) e desenhados
// num <canvas> 2D (context2d), interpolados quadro a quadro (crossfade) — suave em qualquer velocidade.
import { faseDoMotor } from './motor_wasm.js'   // o relógio ÚNICO (o painel_motor.wasm do chessc)
import { velEstado } from './vel_estado.js'     // o slider (ω, play/pause)
import { registra } from './relogio.js'         // o relógio ÚNICO — os cards leem a fase já avançada (sem atraso)

const players = []

class Player {
  constructor (img) {
    this.img = img
    this.frames = null; this.durs = null       // durs mantido só para o decode; o avanço é por fase/2π
    this.cv = null; this.ctx = null
    this.idx = 0; this._frac = -1
    this.decoding = false; this.visible = false
    // O REI é o DUAL: a transformação dele é ANTIFASE com as demais (a régua do Rei — o gap contínuo). O card
    // do Rei (rei_cifra, em elenco3d/textura/óptico) é um GIF; a antifase, para o GIF, é tocar defasado de meia
    // volta (0.5). Sem isso ele toca no MESMO sinal que os outros e o mapa treme (o modo comum não se cancela).
    this.antifase = (img.getAttribute('src') || img.getAttribute('data-src') || '').includes('rei_cifra') ? 0.5 : 0
  }

  async ensure () {                            // decodifica ao ficar visível (lazy — controla a memória)
    if (this.frames || this.decoding) return
    this.decoding = true
    try {
      const buf = await (await fetch(this.img.src)).arrayBuffer()
      const dec = new ImageDecoder({ data: buf, type: 'image/gif' })
      await dec.tracks.ready
      await dec.completed
      const n = dec.tracks.selectedTrack.frameCount
      const frames = []
      for (let i = 0; i < n; i++) {
        const r = await dec.decode({ frameIndex: i })
        frames.push(await createImageBitmap(r.image))
        r.image.close()
      }
      const cv = document.createElement('canvas')
      cv.width = frames[0].width; cv.height = frames[0].height
      cv.className = this.img.className
      cv.setAttribute('aria-label', this.img.alt || '')
      this.frames = frames; this.cv = cv; this.ctx = cv.getContext('2d')
      this.img.replaceWith(cv)
      this.visible = true
      this._draw(0, 0)
    } catch (e) { /* deixa o <img> como está (fallback nativo) */ }
    this.decoding = false
  }

  _draw (idx, frac) {                          // context2d pixel a pixel: o quadro + o crossfade com o próximo
    const n = this.frames.length
    this.ctx.globalAlpha = 1
    this.ctx.drawImage(this.frames[idx], 0, 0)
    if (frac > 0.004) {
      this.ctx.globalAlpha = frac
      this.ctx.drawImage(this.frames[(idx + 1) % n], 0, 0)
      this.ctx.globalAlpha = 1
    }
  }

  advance () {
    if (!this.frames || !this.visible) return
    // 1 ciclo por VOLTA (faseDoMotor é a fração [0,1), bit a bit) — sincronizado com o coração-relógio; o wrap
    // é contínuo e EXATO (frac: 1-ε → 0, sem período incomensurável, sem gap sujo). Os cards seguem o dipolo.
    const n = this.frames.length
    const fase = (faseDoMotor() + this.antifase) % 1        // o REI em ANTIFASE (0.5); os demais em fase (0)
    const x = fase * n
    const idx = ((Math.floor(x) % n) + n) % n
    const frac = x - Math.floor(x)
    if (idx !== this.idx || Math.abs(frac - this._frac) > 0.01) { this.idx = idx; this._frac = frac; this._draw(idx, frac) }
  }
}

function controle () {
  const box = document.createElement('div')
  box.className = 'velctl'
  box.innerHTML = `
    <button class="velplay" aria-label="pausar / tocar" title="pausar / tocar o motor">⏸</button>
    <input class="velrange" type="range" min="0.25" max="4" step="0.05" value="2" aria-label="velocidade do motor">
    <span class="velval" title="a frequência do reino — ω do painel">2.0×</span>
    <span class="velfp" title="o fator de potência do circuito: 1 = todos sincronizados">FP=1</span>`
  const range = box.querySelector('.velrange')
  const val = box.querySelector('.velval')
  const play = box.querySelector('.velplay')
  // COLAPSÁVEL: recolhido é uma pastilha estreita (o relógio + a velocidade). Tocar/clicar no valor abre a
  // régua (o slider). No desktop o hover já abre; no telemóvel, o toque — e um clique fora fecha.
  val.setAttribute('role', 'button')
  val.setAttribute('tabindex', '0')
  val.title = 'abrir a régua da velocidade'
  const alterna = (e) => { e.stopPropagation(); box.classList.toggle('aberto') }
  val.addEventListener('click', alterna)
  val.addEventListener('keydown', (e) => { if (e.key === 'Enter' || e.key === ' ') alterna(e) })
  document.addEventListener('click', (e) => { if (!box.contains(e.target)) box.classList.remove('aberto') })
  range.addEventListener('input', () => { velEstado.v = parseFloat(range.value); val.textContent = velEstado.v.toFixed(2) + '×' })
  play.addEventListener('click', () => {
    velEstado.tocando = !velEstado.tocando
    play.textContent = velEstado.tocando ? '⏸' : '▶'
    box.classList.toggle('parado', !velEstado.tocando)
  })
  return box
}

export function initVelocidade () {
  const nav = document.querySelector('.nav')
  const box = controle()
  if (nav) nav.appendChild(box); else document.body.appendChild(box)

  if (!('ImageDecoder' in window)) return       // fallback: os <img> tocam nativos (sem o circuito)

  const imgs = [...document.querySelectorAll('.card img[src$=".gif"], .screen img[src$=".gif"]')]
  imgs.forEach((img) => players.push(new Player(img)))

  // decodifica os VISÍVEIS (lazy, em paralelo) via getBoundingClientRect — robusto (sem depender do IO)
  const atualiza = () => {
    const vh = window.innerHeight || document.documentElement.clientHeight
    for (const p of players) {
      const t = p.cv || p.img
      if (!t || !t.isConnected) continue
      const r = t.getBoundingClientRect()
      const vis = r.width > 0 && r.bottom > -240 && r.top < vh + 240
      p.visible = vis
      if (vis) p.ensure()
    }
  }
  let raf = 0
  const agenda = () => { if (!raf) raf = requestAnimationFrame(() => { raf = 0; atualiza() }) }
  window.addEventListener('scroll', agenda, { passive: true })
  window.addEventListener('resize', agenda, { passive: true })
  atualiza()
  setInterval(atualiza, 1200)
  registra(() => { for (const p of players) p.advance() })   // os cards avançam no relógio único (a fase já pronta)
}
