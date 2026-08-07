// ── O RELÓGIO — um SÓ loop rAF, o caminho LIVRE (sem bufferização, sem atraso entre etapas) ──
// O impedimento antigo: três loops rAF separados (o painel, o motor o campo, os players) — um renderizador
// podia ler a fase de um tick anterior (1 frame de atraso = bufferização). Aqui há UM só fluxo por quadro:
//   1. a FASE avança (o painel do motor, no topo)   →   2. os CONSUMIDORES leem a fase JÁ atualizada.
// Nada entre eles: nem buffer, nem transformação adicional. A fase computada num quadro é consumida no mesmo.
const consumidores = []

export function registra (fn) { consumidores.push(fn) }

export function iniciaRelogio (avancaFase) {
  let last = 0
  function frame (ts) {
    const dt = last ? Math.min(ts - last, 100) / 1000 : 0.016
    last = ts
    avancaFase(dt)                          // 1. a fase avança (o painel_motor.wasm) — o topo do fluxo
    for (let i = 0; i < consumidores.length; i++) consumidores[i]()   // 2. os consumidores, a fase já pronta
    requestAnimationFrame(frame)
  }
  requestAnimationFrame(frame)
}
