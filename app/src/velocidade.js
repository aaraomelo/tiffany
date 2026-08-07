// ── O CONTROLE DE VELOCIDADE — o circuito síncrono dos cards (context2d, pixel a pixel) ──
// O slider é ω (a referência do painel). O RELÓGIO ÚNICO (a fase do painel_motor.wasm, faseDoMotor) dirige
// TODOS os cards: cada um completa 1 CICLO por 2π de fase — a mesma fase do coração (cos). Assim o wrap
// (2π→0) é CONTÍNUO para todos (idx n→0, o crossfade emenda), sem a descontinuidade dos períodos
// incomensuráveis. (sem gravações a descodificar — só os visíveis, em paralelo) e desenhados
// num <canvas> 2D (context2d), interpolados quadro a quadro (crossfade) — suave em qualquer velocidade.
import { faseDoMotor } from './motor_wasm.js'   // o relógio ÚNICO (o painel_motor.wasm do chessc)
import { velEstado } from './vel_estado.js'     // o slider (ω, play/pause)
import { registra } from './relogio.js'         // o relógio ÚNICO — os cards leem a fase já avançada (sem atraso)

const players = []

// O Player de gravações saiu com as gravações: não há ficheiro a descodificar.

export function initVelocidade () {
  const nav = document.querySelector('.nav')
  const box = controle()
  if (nav) nav.appendChild(box); else document.body.appendChild(box)

  // Os players de gravação saíram com as gravações: não há ficheiro a descodificar, e o que
  // resta é o CONTROLE — o slider é ω, e ele entra no relógio único como sempre entrou.

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
