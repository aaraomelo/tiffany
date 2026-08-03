// ── O PAINEL DE CONTROLE DO MOTOR — o painel_motor.wasm + o torque_fractal.wasm (do CHESSC), no front ──
// Nada por cima: os dois .wasm foram emitidos pelo chessc (o broca-so) de naked Chess (a IR ERG-64), a
// costura interp≡WASM≡Dafny provada. O AVANÇO da fase (painel_motor.wasm): nova_fase = fase + ω·dt (a MULT
// ⊗ da Rainha ∘ a SOMA ⊕ de Kirchhoff). O TORQUE FRACTAL (torque_fractal.wasm): a torre de Koch Σφ^-j (os
// harmônicos de Fibonacci) — a modulação multinível que fractaliza o pulso (lida direto dos slots, sem
// transformação). O slider é ω. A fase (o relógio do reino) NÃO tem rAF próprio: o RELÓGIO ÚNICO a avança
// no topo do quadro (avancaFase) e os consumidores a leem já pronta — o caminho livre, sem bufferização.
import { velEstado } from './vel_estado.js'
import assinaturaTorque from './assinatura_torque.json'   // a torre de Koch ÁUREA (φ^-j de ℤ[φ], exato — o float só aqui)

// A RÉGUA BIT A BIT (resíduo 0): a fase é a FRAÇÃO da volta, [0,1) em ponto fixo 2^20. Como 2^20 é potência de
// 2, o wrap é um AND (& MASK) — EXATO, o ciclo FECHA sem resíduo. O 2π NÃO entra aqui (seria vazamento: 2π é
// irracional, round(2π·2^20) sobra 0.317, sujando o gap). O 2π aparece só no cos(2π·frac), no shader — uma
// escala, nunca um módulo. Assim o gap que sobra é o VERDADEIRO (o do coração-relógio), não a sujeira da conversão.
const ESCALA = 1 << 20                                  // a fração em ponto fixo (2^20)
const MASK = BigInt(ESCALA - 1)                         // o wrap bit a bit (AND) — o ciclo unitário fecha exato
const VEL_BASE = 0.15                                   // voltas/s com o slider em 1× (a frequência-base do reino)
let painel = null                                       // { view: BigInt64Array, prog } — o painel_motor.wasm
let faseFixo = 0n

// a TORRE DE KOCH ÁUREA (a assinatura da dinâmica): os pesos φ^-j vêm do corpo áureo ℤ[φ] (Fibonacci, EXATO),
// projetados para float SÓ aqui (o fim). A cena é livre; o motor (que a revela pela assinatura) é exato — a torre
// é o motor, então é a régua áurea (não o ponto fixo aproximado). torre = Σφ^-j = φ² = (1,1) em ℤ[φ].
const kochAmps = assinaturaTorque.amps                  // φ^-j de ℤ[φ] projetado (torque_aureo_app.py, resíduo 0)
const KOCH_FREQS = assinaturaTorque.freqs               // os k_j (Fibonacci — os harmônicos da garrafa de Koch)

// a MULT ⊗ do painel (double-and-add) em ALTA precisão: prod = omegaF · dtF (fase=0 — só o produto). O painel
// faz a Rainha (⊗) exata sobre i64; passamos ω e dt em ponto fixo 2^20 cada, então prod = ω·dt·2^40 (exato).
function multFixo (omegaF, dtF) {
  const v = painel.view
  const w = BigInt(omegaF)
  v[0] = 0n; v[1] = 0n; v[2] = w; v[3] = BigInt(dtF); v[4] = 0n
  v[5] = 0n; v[6] = 1n; v[7] = w; v[8] = 1n; v[9] = 64n; v[10] = 0n   // reseta prod/mask/aa/one/cnt/tmp
  painel.prog()
  return v[4]                                          // = ω·dt·2^40 (a MULT ⊗, alta precisão — sem a parasita)
}

export async function initMotorWasm () {
  try {
    const { instance } = await WebAssembly.instantiate(await (await fetch('/wasm/painel_motor.wasm')).arrayBuffer())
    painel = { view: new BigInt64Array(instance.exports.mem.buffer), prog: instance.exports.prog }
  } catch (e) { painel = null }        // fallback: o avanço em JS (o mesmo fase += ω·dt)
  return !!painel                      // a torre de Koch vem da assinatura áurea (ℤ[φ], o JSON) — a régua exata
}

// o RELÓGIO ÚNICO chama isto no topo de cada quadro — avança a fase (o painel do chessc), sem rAF próprio.
// ω = velEstado.v · VEL_BASE (voltas/s); o incremento é em FRAÇÃO da volta; o wrap é bit a bit (& MASK, exato).
// SEM A FREQUÊNCIA PARASITA: ω e dt entram em ponto fixo 2^20 (dt·2^20 ≈ 17476, resolução fina — não o antigo
// dt·E/1000 ≈ 17, que saltava 17↔18 com o jitter do rAF e modulava a velocidade ±3%). A MULT ⊗ (o painel) dá
// ω·dt·2^40; o >>20 (renormalização, arredondado) e a SOMA ⊕ + o wrap são bit a bit no JS (resíduo 0).
const MEIO = 1n << 19n                                 // o arredondamento do shift >>20 (sem viés, sem parasita)
export function avancaFase (dt) {
  if (!velEstado.tocando) return
  const omega = velEstado.v * VEL_BASE               // o slider é o × da frequência-base (voltas/s)
  const of = Math.round(omega * ESCALA)              // ω em ponto fixo 2^20 (alta precisão)
  const df = Math.round(dt * ESCALA)                 // dt em ponto fixo 2^20 (~17476: resolução fina, sem salto)
  if (painel) {
    const prod = multFixo(of, df)                    // ω·dt·2^40 (a MULT ⊗ do chessc, exata)
    const inc = (prod + MEIO) >> 20n                 // renormaliza para ω·dt·2^20 (o >>20 bit a bit, arredondado)
    faseFixo = (faseFixo + inc) & MASK               // a SOMA ⊕ + o wrap BIT A BIT: o ciclo fecha exato (resíduo 0)
  } else {
    faseFixo = (faseFixo + ((BigInt(of) * BigInt(df) + MEIO) >> 20n)) & MASK
  }
}

// a FRAÇÃO da volta [0,1) que o painel do chessc computou — exata em double (2^20 cabe na mantissa). O shader
// faz cos(2π·frac); os cards fazem frac·n. O 2π/o ciclo nunca são módulo aqui — só o AND bit a bit (sem gap sujo).
export function faseDoMotor () { return Number(faseFixo) / ESCALA }
export function temWasm () { return !!painel }

// a TORRE DE KOCH (o torque fractal, do chessc) — os pesos φ^-j e os harmônicos, para o shader modular o pulso
export function kochTorre () { return { amps: kochAmps, freqs: KOCH_FREQS } }
