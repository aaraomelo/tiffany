// ── O CARD DA BATALHA EM CENA — em tempo real, no circuito (sai o GIF) ──
// A assinatura não foi inventada: está certificada (assinatura_cards_cena.py, 4/4, resíduo 0).
//   A BATALHA é a CAPTURA, e a captura É ROTAÇÃO: A=cos²θ e B=sin²θ — a energia TRANSFERE de um ao outro (a
//   batalha acontece), mas A+B=1 SEMPRE (s²+c²=1, Parseval). Nada se perde: é o MATE QUE NÃO MATA.
// Corre no RELÓGIO-MESTRE (o mesmo faseDoMotor, bit a bit) — está no circuito, e o slider a comanda.
// Os PÉGASOS (elenco, elenco 3D, texturização, elenco óptico) ficam com a ARTE ORIGINAL: essas seções são
// padrão — mostram a arte da peça, não a dinâmica dela.
import { faseDoMotor } from './motor_wasm.js'
import { registra } from './relogio.js'
import ass from './assinatura_cards.json'

const VS = `#version 300 es
in vec2 pos; void main(){ gl_Position = vec4(pos, 0.0, 1.0); }`

const COMUM = `#version 300 es
precision highp float;
uniform vec2 u_res; uniform float u_time;
out vec4 o;
const float TAU = 6.283185307179586;
vec3 ceu(vec3 rd){ float t=clamp(rd.z*0.5+0.5,0.0,1.0);
  return mix(vec3(0.04,0.04,0.08), vec3(0.09,0.13,0.28), t) + exp2(-abs(rd.z)*5.0)*vec3(0.55,0.35,0.20); }
float sdEsf(vec3 p, float r){ return length(p)-r; }
// o ANEL S¹ (a régua circular do rotor): o toro no plano xy
float sdAnel(vec3 p, float R, float r){ vec2 q=vec2(length(p.xy)-R, p.z); return length(q)-r; }
mat2 rot(float a){ float c=cos(a), s=sin(a); return mat2(c,-s,s,c); }
`

// A BATALHA — a CAPTURA É ROTAÇÃO: A=cos²θ, B=sin²θ. A energia transfere; a soma A+B=1 conserva (nada se perde)
const FS_BATALHA = COMUM + `
uniform vec3 u_corA; uniform vec3 u_corB; uniform vec3 u_corElo; uniform float u_sep;
float mapa(vec3 p, float a, float b, out float qual){
  // os raios são as ENERGIAS: r ∝ cbrt(energia) (o volume da esfera é a energia — a régua honesta)
  float rA = 0.30 + 0.55*pow(a, 0.3333), rB = 0.30 + 0.55*pow(b, 0.3333);
  float eA = sdEsf(p - vec3(-u_sep,0.0,0.0), rA);
  float eB = sdEsf(p - vec3( u_sep,0.0,0.0), rB);
  // o ELO: a energia TRAFEGA encapsulada de um ao outro (o fio; nada escapa — a captura não destrói)
  vec3 q1=p-vec3(-u_sep,0.0,0.0), ba=vec3(2.0*u_sep,0.0,0.0);
  float h=clamp(dot(q1,ba)/dot(ba,ba),0.0,1.0);
  float elo = length(q1-ba*h) - 0.045;
  qual = elo < min(eA,eB) ? 2.0 : (eA < eB ? 0.0 : 1.0);  // 2=o elo · 0=A · 1=B
  return min(min(eA,eB), elo);
}
vec3 nrm(vec3 p, float a, float b){ const vec2 k=vec2(1.0,-1.0); const float e=1.5e-3; float q;
  return normalize(k.xyy*mapa(p+k.xyy*e,a,b,q) + k.yyx*mapa(p+k.yyx*e,a,b,q)
                 + k.yxy*mapa(p+k.yxy*e,a,b,q) + k.xxx*mapa(p+k.xxx*e,a,b,q)); }
void main(){
  vec2 uv=(gl_FragCoord.xy*2.0 - u_res)/u_res.y;
  vec3 ro=vec3(0.0,4.3,1.0), ta=vec3(0.0,0.0,0.0);
  vec3 ww=normalize(ta-ro), uu=normalize(cross(ww,vec3(0.,0.,1.))), vv=cross(uu,ww);
  vec3 rd=normalize(uv.x*uu + uv.y*vv + 1.55*ww);
  float th = TAU*u_time;
  float a = cos(th)*cos(th), b = sin(th)*sin(th);        // A+B=1 SEMPRE (Parseval) — nada se perde
  vec3 col=ceu(rd); float t=0.0; float q=0.0;
  for(int i=0;i<70;i++){ vec3 p=ro+t*rd; float d=mapa(p,a,b,q);
    if(d<0.0015){
      vec3 n=nrm(p,a,b); vec3 L=normalize(vec3(0.45,0.55,0.85)), v=-rd, h=normalize(L+v);
      float dif=max(dot(n,L),0.0), spec=pow(max(dot(n,h),0.0),44.0), fres=pow(1.0-max(dot(n,v),0.0),4.0);
      vec3 base = q>1.5 ? u_corElo : (q<0.5 ? u_corA : u_corB);
      float emis = 0.0;
      if(q>1.5){                                          // o ELO brilha onde a energia PASSA (o ponteiro)
        float hp=clamp((p.x+u_sep)/(2.0*u_sep),0.0,1.0);
        emis = 0.8*pow(0.5+0.5*cos(TAU*(hp - u_time)),8.0);
      }
      col = base*(0.32+0.85*dif) + spec*vec3(0.95) + fres*vec3(0.35,0.5,0.75)
          + 0.18*ceu(reflect(rd,n)) + emis*u_corElo;
      break; }
    t += max(d*0.8, 0.006); if(t>10.0) break; }
  o=vec4(pow(clamp(col,0.0,1.0), vec3(0.87)), 1.0);
}`

function montar (host, fs, setUniforms) {
  const canvas = document.createElement('canvas')
  const gl = canvas.getContext('webgl2', { antialias: true, alpha: false })
  if (!gl) return false                                    // sem WebGL2: fica o GIF (fallback)
  const compile = (type, src) => {
    const s = gl.createShader(type); gl.shaderSource(s, src); gl.compileShader(s)
    if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) { console.warn(gl.getShaderInfoLog(s)); return null }
    return s
  }
  const vs = compile(gl.VERTEX_SHADER, VS), f = compile(gl.FRAGMENT_SHADER, fs)
  if (!vs || !f) return false
  const prog = gl.createProgram(); gl.attachShader(prog, vs); gl.attachShader(prog, f); gl.linkProgram(prog)
  if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) { console.warn(gl.getProgramInfoLog(prog)); return false }
  gl.useProgram(prog)
  const quad = gl.createBuffer(); gl.bindBuffer(gl.ARRAY_BUFFER, quad)
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-1, -1, 1, -1, -1, 1, 1, 1]), gl.STATIC_DRAW)
  const loc = gl.getAttribLocation(prog, 'pos'); gl.enableVertexAttribArray(loc)
  gl.vertexAttribPointer(loc, 2, gl.FLOAT, false, 0, 0)
  const uRes = gl.getUniformLocation(prog, 'u_res'), uTime = gl.getUniformLocation(prog, 'u_time')
  setUniforms(gl, prog)                                    // a ASSINATURA (do JSON certificado) vira uniforms

  const img = host.querySelector('img')                    // sai o GIF, entra a cena
  if (img) { canvas.className = img.className; canvas.setAttribute('aria-label', img.alt || ''); img.replaceWith(canvas) }
  else host.appendChild(canvas)

  function render () {                                     // sem rAF próprio: o RELÓGIO ÚNICO o chama
    const r = canvas.getBoundingClientRect()
    if (!(r.width > 0 && r.bottom > -120 && r.top < (window.innerHeight || 800) + 120)) return
    const w = Math.max(2, Math.round(r.width * Math.min(devicePixelRatio, 1.5)))
    const h = Math.max(2, Math.round(r.height * Math.min(devicePixelRatio, 1.5)))
    if (canvas.width !== w || canvas.height !== h) { canvas.width = w; canvas.height = h }
    gl.viewport(0, 0, canvas.width, canvas.height)
    gl.uniform2f(uRes, canvas.width, canvas.height)
    gl.uniform1f(uTime, faseDoMotor())                     // o relógio-mestre — a cena está NO CIRCUITO
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4)
  }
  registra(render)
  render()                                                 // o primeiro quadro JÁ (o canvas não entra preto)
  return true
}

export function initCardsGlsl () {
  const b = ass.batalha
  // OS PÉGASOS FICAM COMO ESTÃO — a arte original, em TODAS as seções (elenco, elenco 3D, texturização, elenco
  // óptico). Essas seções são padrão: mostram a arte da peça, não a dinâmica dela. Trocar a geometria delas foi
  // erro meu (a confusão). Só a BATALHA (e o trailer) viram cena em tempo real.
  // A BATALHA (a captura é rotação) — o card da dinâmica
  // o card nasce com data-src (sem src: o GIF do modelo antigo não é baixado nem pisca) — casa-se pelos dois
  document.querySelectorAll('.card img[data-src*="captura"], .card img[src*="captura"]').forEach((img) => {
    const frame = img.closest('.frame') || img.parentElement
    montar(frame, FS_BATALHA, (gl, prog) => {
      gl.uniform3fv(gl.getUniformLocation(prog, 'u_corA'), new Float32Array(b.corA))
      gl.uniform3fv(gl.getUniformLocation(prog, 'u_corB'), new Float32Array(b.corB))
      gl.uniform3fv(gl.getUniformLocation(prog, 'u_corElo'), new Float32Array(b.corElo))
      gl.uniform1f(gl.getUniformLocation(prog, 'u_sep'), b.sep)
    })
  })
}
