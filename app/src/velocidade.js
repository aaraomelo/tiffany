// ── O CONTROLE DE VELOCIDADE — o slider é ω, a referência do painel ──
// O RELÓGIO ÚNICO (a fase do painel_motor.wasm, faseDoMotor) dirige TODOS os cards: cada um completa
// 1 CICLO por 2π de fase — a mesma fase do coração (cos). Assim o wrap (2π→0) é CONTÍNUO para todos,
// sem a descontinuidade dos períodos incomensuráveis.
//
// Este ficheiro é SÓ o controle. Os cards não passam por aqui: entram no relógio pelo seu próprio
// caminho (initCardsCampo, initCardsKernel), e o que o slider faz é mover ω — que é o que o relógio lê.
import { velEstado } from './vel_estado.js'     // o slider (ω, play/pause)

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
}
