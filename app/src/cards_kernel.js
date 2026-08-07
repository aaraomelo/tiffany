// ── OS CARDS SÃO O KERNEL — em tempo real, no motor; sem gravação ──
// O kernel NÃO foi escrito aqui: vem EMITIDO pela BROCA (chessc.campo_kernel), do MESMO PTX que roda no metal.
// Uma régua só — a do sistema. Este arquivo é o operador: liga o kernel emitido ao relógio, à bateria e ao card.
//
//   · a AMPLITUDE (o `a` do kernel, o param `pa` do metal) vem do MOTOR: o pulso da bateria de Koch áurea
//     (φ^-j, Fibonacci) na fase do relógio-mestre — não de um laço de gravação (senoide de fora = harmônico que
//     não é da bateria = distorção = FP<1).
//   · a JANELA (x0, dx) é a do metal, invariante à resolução: dx = 2·lim/W. O campo é do contínuo — em
//     qualquer tamanho de card, o pixel é computado, não interpolado.
//   · a PALETA são os 5 stops do metal, interpolados ANALITICAMENTE no kernel (sem textura — a broca é ALU pura).
//   · UM contexto WebGL para todos: o navegador só mantém ~16 vivos, e 27 canvas derrubavam-nos (medido).
import { faseDoMotor, kochTorre } from './motor_wasm.js'
import { registra } from './relogio.js'
import art from './kernels_campo.json'

const VS = `#version 300 es
in vec2 pos; void main(){ gl_Position = vec4(pos, 0.0, 1.0); }`

const mesa = { gl: null, quad: null, progs: new Map(), pecas: [] }

function contexto () {
  if (mesa.gl) return mesa.gl
  const cv = document.createElement('canvas')
  const gl = cv.getContext('webgl2', { antialias: false, alpha: true, premultipliedAlpha: true })
  if (!gl) return null   // sem GPU o SHADING não corre — mas o corpo (a fase) corre na mesma:
                         // quem desenha é roupa, quem conta as voltas é o relógio, e esse é wasm
  mesa.gl = gl
  mesa.quad = gl.createBuffer()
  gl.bindBuffer(gl.ARRAY_BUFFER, mesa.quad)
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-1, -1, 1, -1, -1, 1, 1, 1]), gl.STATIC_DRAW)
  gl.enable(gl.BLEND)
  gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA)         // premultiplicado (o alpha do metal)
  return gl
}

function programa (kernel) {                            // compila UMA vez por kernel (não por peça)
  if (mesa.progs.has(kernel)) return mesa.progs.get(kernel)
  const gl = contexto()
  const compila = (tipo, src) => {
    const s = gl.createShader(tipo); gl.shaderSource(s, src); gl.compileShader(s)
    if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) { console.warn('[' + kernel + ']', gl.getShaderInfoLog(s)); return null }
    return s
  }
  const vs = compila(gl.VERTEX_SHADER, VS)
  const fs = compila(gl.FRAGMENT_SHADER, art.kernels[kernel])   // o kernel EMITIDO PELA BROCA
  if (!vs || !fs) return null
  const prog = gl.createProgram(); gl.attachShader(prog, vs); gl.attachShader(prog, fs); gl.linkProgram(prog)
  if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) { console.warn('[' + kernel + ']', gl.getProgramInfoLog(prog)); return null }
  mesa.progs.set(kernel, prog)
  return prog
}

// o PULSO DO MOTOR — a bateria de Koch áurea modula (o mesmo de motor_campo.js). É daqui que sai o `a` do kernel:
// a amplitude do atrator não é um cos de laço; é o batimento do reino.
function pulsoDoMotor () {
  const t = kochTorre()
  const ang = 2 * Math.PI * faseDoMotor()
  let harm = 0
  for (let j = 0; j < t.amps.length; j++) harm += t.amps[j] * Math.cos(t.freqs[j] * ang)
  return Math.min(1, Math.max(0, 0.5 - 0.5 * Math.cos(ang) + 0.03 * harm))
}

// o MOTOR do clock — a seta do tempo: o mesmo sopro fractal, mas com a fase EMPENADA (t^1.7). É irreversível
// (v(t)≠v(1-t)), mas ainda emenda no laço (0→0). O rotor usa o pulso simétrico; o motor, este.
function pulsoWarp () {
  const t = kochTorre()
  const ang = 2 * Math.PI * Math.pow(faseDoMotor(), 1.7)
  let harm = 0
  for (let j = 0; j < t.amps.length; j++) harm += t.amps[j] * Math.cos(t.freqs[j] * ang)
  return Math.min(1, Math.max(0, 0.5 - 0.5 * Math.cos(ang) + 0.03 * harm))
}

// uma ORBE (a AURA de um operador — o kernel `aura`, NÃO uma foto), computada num quadrado (vx, vy, lado) e composta
// por SCREEN sobre o que já está no card. O alpha do campo já a circulariza (a aura decai radialmente). É o tecido
// que se sobrepõe: na cena, o operador sobre o chão; no clock, o rotor e o motor lado a lado.
function desenhaOrbe (orbe, vx, vy, lado, amp) {
  const gl = mesa.gl
  const prog = mesa.progs.get('aura') || programa('aura')
  if (!prog) return
  gl.useProgram(prog)
  gl.bindBuffer(gl.ARRAY_BUFFER, mesa.quad)
  const loc = gl.getAttribLocation(prog, 'pos')
  gl.enableVertexAttribArray(loc); gl.vertexAttribPointer(loc, 2, gl.FLOAT, false, 0, 0)
  gl.viewport(vx, vy, lado, lado)
  const U = (n) => gl.getUniformLocation(prog, n)
  gl.uniform2f(U('u_res'), lado, lado)
  const lim = art.pintor.aura.lim
  gl.uniform1f(U('px0'), -lim); gl.uniform1f(U('py0'), -lim)
  gl.uniform1f(U('pdx'), (2 * lim) / lado); gl.uniform1f(U('pdy'), (2 * lim) / lado)
  gl.uniform1f(U('pa'), amp)
  gl.uniform1f(U('pf'), orbe.f); gl.uniform1i(U('pn'), orbe.n)
  gl.uniform1f(U('pchi'), orbe.chi); gl.uniform1f(U('pw'), orbe.w)
  gl.uniform3fv(U('u_stops'), new Float32Array(orbe.stops.flat()))
  gl.uniform1f(U('u_amp'), 1.0)
  gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_COLOR)         // SCREEN: a orbe soma-se ao fundo (o operador manifesto)
  gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4)
  gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA)         // restaura o premultiplicado (os próximos cards)
  gl.viewport(0, 0, gl.canvas.width, gl.canvas.height)
}

// o CLOCK — duas auras do Pégaso lado a lado: o ROTOR (tempo simétrico, reversível) e o MOTOR (a seta do tempo,
// t^1.7). Não há kernel de fundo: só o par de orbes sobre o card. A assimetria temporal É a diferença entre elas.
function desenhaClock (it, w, h) {
  const gl = mesa.gl
  gl.clearColor(0, 0, 0, 0); gl.clear(gl.COLOR_BUFFER_BIT)
  const lado = Math.round(0.40 * w)
  const cy = Math.round((h - lado) / 2)
  desenhaOrbe(it.p.gemeas, Math.round(0.06 * w), cy, lado, pulsoDoMotor())   // o rotor (simétrico)
  desenhaOrbe(it.p.gemeas, w - lado - Math.round(0.06 * w), cy, lado, pulsoWarp())   // o motor (a seta do tempo)
}

export function initCardsKernel () {
  const gl = contexto()
  if (!gl) return false
  document.querySelectorAll('.frame[data-peca]').forEach((frame) => {
    const p = art.pecas[frame.dataset.peca]
    if (!p || !art.kernels[p.kernel] || !programa(p.kernel)) return
    const cv = document.createElement('canvas')
    frame.appendChild(cv)
    const stops = new Float32Array((p.stops || []).flat())   // os 5 stops (a paleta) — interpolados no kernel
    mesa.pecas.push({ cv, ctx: cv.getContext('2d'), frame, p, stops })
  })
  if (!mesa.pecas.length) return false
  registra(quadro)                                      // entra no RELÓGIO ÚNICO (a fase já pronta)
  quadro()                                              // o primeiro quadro JÁ (o card não entra preto)
  return true
}

function quadro () {
  const gl = mesa.gl
  if (!gl || gl.isContextLost()) return
  const a = pulsoDoMotor()                              // o `a` do kernel: o pulso do MOTOR
  const dpr = Math.min(devicePixelRatio, 1.5)
  for (const it of mesa.pecas) {
    const r = it.frame.getBoundingClientRect()
    if (!(r.width > 0 && r.bottom > -120 && r.top < (window.innerHeight || 800) + 120)) continue
    const w = Math.max(2, Math.round(r.width * dpr)), h = Math.max(2, Math.round(r.height * dpr))
    if (it.cv.width !== w || it.cv.height !== h) { it.cv.width = w; it.cv.height = h }
    if (gl.canvas.width !== w || gl.canvas.height !== h) { gl.canvas.width = w; gl.canvas.height = h }
    const p = it.p
    if (p.gemeas) {                                      // o CLOCK: o par de orbes (rotor ⋈ motor), sem kernel de fundo
      desenhaClock(it, w, h)
      it.ctx.clearRect(0, 0, w, h); it.ctx.drawImage(gl.canvas, 0, 0)
      continue
    }
    const pin = art.pintor[p.kernel]
    const prog = mesa.progs.get(p.kernel)
    gl.useProgram(prog)
    gl.bindBuffer(gl.ARRAY_BUFFER, mesa.quad)
    const loc = gl.getAttribLocation(prog, 'pos')
    gl.enableVertexAttribArray(loc)
    gl.vertexAttribPointer(loc, 2, gl.FLOAT, false, 0, 0)
    gl.viewport(0, 0, w, h)
    const U = (n) => gl.getUniformLocation(prog, n)
    gl.uniform2f(U('u_res'), w, h)
    const eh3d = !pin.lim   // um raymarch 3D (sem janela 2D)? — o inversor é 3D E usa stops (a LUT da cifra)
    if (eh3d) {
      // LIGA o modelo no CIRCUITO: o MOTOR dá o sopro. A fase do motor (u_time) e a bateria de Koch entram; o
      // pulso fractal (a respiração) é calculado no KERNEL (o motor), não em JS. O modelo só respira no circuito.
      gl.uniform1f(U('u_time'), faseDoMotor())
      const t = kochTorre()
      gl.uniform1fv(U('u_kochAmp'), new Float32Array(t.amps))
      gl.uniform1fv(U('u_kochFreq'), new Float32Array(t.freqs))
      // as cores (a assinatura do card): o respira tem 1 (cor); a espiral tem 3 (lo/mid/hi)
      if (p.cor) { gl.uniform1f(U('cr'), p.cor[0]); gl.uniform1f(U('cg'), p.cor[1]); gl.uniform1f(U('cb'), p.cor[2]) }
      if (p.lo) {
        gl.uniform1f(U('lor'), p.lo[0]); gl.uniform1f(U('log'), p.lo[1]); gl.uniform1f(U('lob'), p.lo[2])
        gl.uniform1f(U('mir'), p.mid[0]); gl.uniform1f(U('mig'), p.mid[1]); gl.uniform1f(U('mib'), p.mid[2])
        gl.uniform1f(U('hir'), p.hi[0]); gl.uniform1f(U('hig'), p.hi[1]); gl.uniform1f(U('hib'), p.hi[2])
      }
      // o CÉU da cena: o gradiente do horizonte (hor) ao topo (top) — as cores do corte
      if (p.hor) {
        gl.uniform3f(U('u_hor'), p.hor[0], p.hor[1], p.hor[2])
        gl.uniform3f(U('u_top'), p.top[0], p.top[1], p.top[2])
      }
    } else {
      // a JANELA do metal (2D): x0 = -lim, dx = 2·lim/W — a MESMA régua, em qualquer resolução (é do contínuo)
      gl.uniform1f(U('px0'), -pin.lim); gl.uniform1f(U('py0'), -pin.lim)
      gl.uniform1f(U('pdx'), (2 * pin.lim) / w); gl.uniform1f(U('pdy'), (2 * pin.lim) / h)
      gl.uniform1f(U('pa'), a)                          // a amplitude: do MOTOR
      gl.uniform1f(U('pt'), faseDoMotor())              // o TEMPO (pt): a espiral gira — só o pulso o usa
    }
    // a ASSINATURA da peça: enviada pelo MAPA do artefato (uniforme→chave, com o tipo lido do kernel emitido).
    // Os nomes são os do PTX (pN, pfA, pMax…) — adivinhá-los foi o que matou o julia: pMax não chegava, o laço
    // acabava na iteração 0 e o campo saía 0 (o card do Alonzo, preto).
    const mapa = art.mapa[p.kernel] || {}
    for (const uni in mapa) {
      const [chave, tipo] = mapa[uni]
      let v = p[chave]
      if (v === undefined) continue
      // os kernels sem o param `a` animam por OUTRO uniforme: no julia a amplitude entra no C (a·c_J) — o
      // Julia parte do disco (c=0) e revela o dendrito. É a lei do metal, não uma animação minha.
      if (pin.escala_a && pin.escala_a.includes(uni)) v = v * a
      if (tipo === 'int') gl.uniform1i(U(uni), v); else gl.uniform1f(U(uni), v)
    }
    // o PINTOR do metal: os 5 stops (a paleta), interpolados NO KERNEL (analítico) — e o desvanecer do script
    // a cor (os stops = a assinatura da cor do card) e o alpha: a EQUAÇÃO do metal, embutida no kernel. Os
    // números (ganho, gama…) não são uniforms meus — descem embutidos pela broca. Aqui só a assinatura e o amp.
    if (it.stops.length) gl.uniform3fv(U('u_stops'), it.stops)
    gl.uniform1f(U('u_amp'), 1.0)                       // a transformação do card NÃO se toca (u_amp=1)
    gl.clearColor(0, 0, 0, 0); gl.clear(gl.COLOR_BUFFER_BIT)
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4)
    it.ctx.clearRect(0, 0, w, h)
    it.ctx.drawImage(gl.canvas, 0, 0)                   // o quadro vai do metal para o card
  }
}
