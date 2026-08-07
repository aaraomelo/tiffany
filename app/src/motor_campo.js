// ── O MOTOR EM o campo — o pipe de gênese, em TEMPO REAL no metal do navegador (WebGL2) ──
// O motor é PADRÃO: começa com a GARRAFA DE KOCH VAZIA (a esfera, a=0) e EVOLUI (a→1) pela conversão
// ponto a ponto (o inversor), rasterizada a cada quadro. A ASSINATURA do universo (a forma-alvo, o
// circuito, a métrica) entra como UNIFORMS — traduzida para o campo — e o motor faz o resto. O TEMPO é o
// backend (u_time), com a velocidade do reino (velEstado): é o mesmo relógio do circuito síncrono.
// Roda agora, na GPU, em tempo real — a semente do jogo.
// O u_time NÃO é contado aqui: vem do PAINEL (o DTC em WASM, faseDoMotor) — o WASM (CPU) entrega pro canvas (GPU).
// E o PULSO é FRACTAL: a torre de Koch (Σφ^-j, do torque_fractal.wasm) modula a=0.5-0.5cos(u_time) com os
// harmônicos de Fibonacci — o torque fractal, não a senoidal pura. O render NÃO tem rAF próprio: o RELÓGIO
// ÚNICO o chama depois de a fase já ter avançado — o caminho livre (sem o atraso de quadro entre os loops).
import { faseDoMotor, kochTorre } from './motor_wasm.js'
import { registra } from './relogio.js'

const VS = `#version 300 es
in vec2 pos; void main(){ gl_Position = vec4(pos, 0.0, 1.0); }`

// o motor: o raymarch da conversão esfera→forma-alvo. A ASSINATURA vem nos uniforms (u_c200 = o coeficiente
// da forma do universo; u_luz, u_cor). A garrafa vazia é a=0 (a esfera); a cena é a=1.
const FS = `#version 300 es
precision highp float;
uniform vec2 u_res; uniform float u_time; uniform float u_c200; uniform vec3 u_luz; uniform vec3 u_cor;
uniform vec3 u_corB;                                        // a cor do DUAL (o 2º coração da cena)
uniform float u_kochAmp[6]; uniform float u_kochFreq[6];   // a torre de Koch (φ^-j, Fibonacci) — o torque fractal
uniform float u_faseOff;                                    // a defasagem áurea do dual (φ rad) — o dipolo na MESMA cena
out vec4 o;
const float TAU = 6.283185307179586;                        // 2π — SÓ aqui, na escala do cos (nunca módulo)
const float C225 = 2.25;
float Feq(vec3 p){ float A = p.x*p.x + C225*p.y*p.y + p.z*p.z - 1.0; float z3 = p.z*p.z*p.z;
  return A*A*A - p.x*p.x*z3 - u_c200*p.y*p.y*z3; }
float resf(vec3 d){ return 1.0/sqrt(d.x*d.x + C225*d.y*d.y + d.z*d.z); }
// dF/dr = grad(F)·dir — o gradiente radial, para o NEWTON pousar na RAIZ EXATA da costura (Feq=0)
float dFdr(vec3 d, float r){ vec3 q=r*d; float A=q.x*q.x+C225*q.y*q.y+q.z*q.z-1.0, A2=A*A, z2=q.z*q.z, z3=z2*q.z;
  float gx=6.0*q.x*A2-2.0*q.x*z3, gy=13.5*q.y*A2-2.0*u_c200*q.y*z3, gz=6.0*q.z*A2-3.0*q.x*q.x*z2-3.0*u_c200*q.y*q.y*z2;
  return gx*d.x+gy*d.y+gz*d.z; }
// a COSTURA do coração: BISSEÇÃO (bracket, a raiz certa star-shaped) + NEWTON (a raiz EXATA, resíduo ~épsilon).
// A bisseção-só (o vazamento antigo) parava em ~1e-5; o Newton pousa em Feq=0 (a superfície fecha, sem vazamento).
float rc(vec3 d){ float lo=0.15, hi=1.7;
  for(int i=0;i<18;i++){ float m=0.5*(lo+hi); if(Feq(m*d)<0.0) lo=m; else hi=m; }   // bracket (evita as raízes internas)
  float r=0.5*(lo+hi);
  for(int i=0;i<3;i++){ r -= Feq(r*d)/dFdr(d,r); }                                   // Newton: a raiz exata da costura
  return r; }
float mapaUm(vec3 p, float a){ float rho=length(p); if(rho<1e-4) return -0.5;
  vec3 d=p/rho; float ra=mix(resf(d), rc(d), a); return (rho-ra); }
const float DX = 1.70;                                     // a separação (folga ~1.0 entre eles: o fio do Rei respira)
const float RREI = 0.05;                                   // a espessura do FIO do Rei (fino: uma costura, não uma barra)
const vec3 OURO_REI = vec3(1.0, 0.82, 0.30);               // o dourado do Rei (σ_m, o gap que une os duais)
// o coração A (o Príncipe) em -DX. O coração B (Dark Pontryagin) em +DX, DUAL GEOMÉTRICO: espelhado em z
// (ponta↔lóbulos) — dual em tudo: geometria (z→-z), fase (antifase áurea) e cor (frio ⋈ quente).
float mapaB(vec3 p, float aB){ vec3 q=p-vec3(DX,0.0,0.0); q.z=-q.z; return mapaUm(q, aB); }
// o REI costura os dois: uma cápsula (o fio dourado) de -DX a +DX no plano z=0 — o gap σ_m que liga o par dual
float sdRei(vec3 p){ vec3 a=vec3(-DX,0.0,0.0), b=vec3(DX,0.0,0.0), pa=p-a, ba=b-a;
  float h=clamp(dot(pa,ba)/dot(ba,ba),0.0,1.0); return length(pa-ba*h)-RREI; }
// a cena: os DOIS corações duais + a costura do Rei, na MESMA cena (união = min)
float mapa2(vec3 p, float aA, float aB){
  return min(min(mapaUm(p - vec3(-DX,0.0,0.0), aA), mapaB(p, aB)), sdRei(p)) * 0.6; }
vec3 nrm2(vec3 p, float aA, float aB){ const vec2 k=vec2(1.0,-1.0); const float e=1.6e-3;   // 4 taps (tetraedro)
  return normalize(k.xyy*mapa2(p+k.xyy*e,aA,aB) + k.yyx*mapa2(p+k.yyx*e,aA,aB) + k.yxy*mapa2(p+k.yxy*e,aA,aB) + k.xxx*mapa2(p+k.xxx*e,aA,aB)); }
// o PULSO fractal (a torre de Koch modula) — parametrizado pela defasagem: A (off=0) e B (off=φ, a antifase áurea)
float pulso(float off){ float ang = TAU*u_time + off; float harm=0.0;
  for(int j=0;j<6;j++) harm += u_kochAmp[j]*cos(u_kochFreq[j]*ang);
  return clamp(0.5-0.5*cos(ang) + 0.03*harm, 0.0, 1.0); }
vec3 ceu(vec3 rd){ float t=clamp(rd.z*0.5+0.5,0.0,1.0);
  return mix(vec3(0.04,0.04,0.08), vec3(0.09,0.13,0.28), t) + exp2(-abs(rd.z)*5.0)*vec3(0.55,0.35,0.20); }
void main(){
  vec2 uv=(gl_FragCoord.xy*2.0 - u_res)/u_res.y;
  vec3 ro=vec3(0.25,5.60,0.85), ta=vec3(0.0,0.0,-0.05);     // câmera afastada e acima: enquadra os DOIS + a costura
  vec3 ww=normalize(ta-ro), uu=normalize(cross(ww,vec3(0.,0.,1.))), vv=cross(uu,ww);
  vec3 rd=normalize(uv.x*uu + uv.y*vv + 1.45*ww);           // fov largo (a cena tem ~6 de largura)
  // u_time é a FRAÇÃO da volta [0,1) (o wrap bit a bit, resíduo 0). O TORQUE FRACTAL (torre de Koch) modula o
  // pulso. O DIPOLO: A (fase φ) e B (fase φ + defasagem áurea) — o gap áureo entre os dois move o relógio.
  float aA = pulso(0.0);                              // o coração
  float aB = pulso(u_faseOff);                        // o seu dual, defasado (a antifase áurea φ)
  // BOUNDING SPHERE (raio 3.2): cobre os DOIS corações (x=±1.70, raio ~1.1) e a costura, pula o vazio
  float bb=dot(ro,rd), cc=dot(ro,ro)-10.24;          // 3.2^2
  float disc=bb*bb-cc;
  vec3 col;
  if(disc<=0.0){ col=ceu(rd); }                      // o raio nem toca a esfera envolvente: só o céu
  else{
    float sd=sqrt(disc); float t=max(-bb-sd,0.0), tmax=-bb+sd;   // entra e sai do bounding
    bool hit=false; vec3 p=ro;
    for(int i=0;i<68;i++){ p=ro+t*rd; float d=mapa2(p,aA,aB); if(d<0.001){hit=true;break;} t+=max(d*0.7,0.006); if(t>tmax)break; }
    if(hit){
      vec3 n=nrm2(p,aA,aB); vec3 L=normalize(u_luz), v=-rd, h=normalize(L+v);
      float dif=max(dot(n,L),0.0), spec=pow(max(dot(n,h),0.0),40.0), fres=pow(1.0-max(dot(n,v),0.0),4.0);
      // qual superfície: o Príncipe (u_cor), o Dark dual (u_corB) ou a costura do REI (dourado, O RELÓGIO)
      float dA=mapaUm(p-vec3(-DX,0.0,0.0),aA), dB=mapaB(p,aB), dK=sdRei(p);
      vec3 base; float emis=0.0;
      if(dK<dA && dK<dB){
        // o REI é O RELÓGIO — o único que traz o gap DE FORA: um ponteiro dourado corre pela costura (a fase),
        // e é dele que o gap (a defasagem áurea entre os duais) vem. O fio brilha onde o ponteiro passa.
        float hpos=clamp((p.x+DX)/(2.0*DX),0.0,1.0);
        float pont=pow(0.5+0.5*cos(TAU*(hpos - u_time)),8.0);
        base=OURO_REI; emis=0.85*pont;
      } else base = dA<dB ? u_cor : u_corB;
      col = base*(0.34+0.82*dif) + spec*vec3(0.95) + fres*vec3(0.35,0.5,0.75) + 0.18*ceu(reflect(rd,n)) + emis*OURO_REI;
    } else col = ceu(rd);
  }
  o=vec4(pow(clamp(col,0.0,1.0), vec3(0.87)), 1.0);
}`

// traduz a ASSINATURA (o dict do universo) para os uniforms o campo — automático
function assinaturaParaUniforms (gl, prog, ass) {
  gl.uniform1f(gl.getUniformLocation(prog, 'u_c200'), ass.c200 ?? 0.045)      // a forma-alvo (9/200 = coração)
  gl.uniform3fv(gl.getUniformLocation(prog, 'u_luz'), ass.luz ?? [0.4, 0.55, 0.85])
  gl.uniform3fv(gl.getUniformLocation(prog, 'u_cor'), ass.cor ?? [0.30, 0.62, 0.52])
  gl.uniform3fv(gl.getUniformLocation(prog, 'u_corB'), ass.corB ?? ass.cor ?? [0.30, 0.62, 0.52])   // o dual
  const t = kochTorre()                                                       // a torre de Koch (do torque_fractal.wasm)
  gl.uniform1fv(gl.getUniformLocation(prog, 'u_kochAmp'), new Float32Array(t.amps))
  gl.uniform1fv(gl.getUniformLocation(prog, 'u_kochFreq'), new Float32Array(t.freqs))
}

export function initMotorCampo (host, assinatura = {}, faseOff = 0) {
  const canvas = document.createElement('canvas')
  const gl = canvas.getContext('webgl2', { antialias: true, alpha: false })
  if (!gl) return false                                     // sem WebGL2: fica o <img> (fallback)
  const compile = (type, src) => {
    const s = gl.createShader(type); gl.shaderSource(s, src); gl.compileShader(s)
    if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) { console.warn(gl.getShaderInfoLog(s)); return null }
    return s
  }
  const vs = compile(gl.VERTEX_SHADER, VS), fs = compile(gl.FRAGMENT_SHADER, FS)
  if (!vs || !fs) return false
  const prog = gl.createProgram(); gl.attachShader(prog, vs); gl.attachShader(prog, fs); gl.linkProgram(prog)
  if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) { console.warn(gl.getProgramInfoLog(prog)); return false }
  gl.useProgram(prog)
  const quad = gl.createBuffer(); gl.bindBuffer(gl.ARRAY_BUFFER, quad)
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-1, -1, 1, -1, -1, 1, 1, 1]), gl.STATIC_DRAW)
  const loc = gl.getAttribLocation(prog, 'pos'); gl.enableVertexAttribArray(loc)
  gl.vertexAttribPointer(loc, 2, gl.FLOAT, false, 0, 0)
  const uRes = gl.getUniformLocation(prog, 'u_res'), uTime = gl.getUniformLocation(prog, 'u_time')
  assinaturaParaUniforms(gl, prog, assinatura)
  gl.uniform1f(gl.getUniformLocation(prog, 'u_faseOff'), faseOff)   // 0 = o coração; π = o dual (antifase)

  // substitui o <img> do host pelo canvas (o motor vivo)
  const img = host.querySelector('img')
  if (img) { canvas.className = img.className; canvas.setAttribute('aria-label', img.alt || ''); img.replaceWith(canvas) }
  else host.appendChild(canvas)

  // o render NÃO tem rAF próprio: o RELÓGIO ÚNICO o chama após a fase já ter avançado (o caminho livre)
  function render () {
    const r = canvas.getBoundingClientRect()
    const vis = r.width > 0 && r.bottom > -120 && r.top < (window.innerHeight || 800) + 120
    if (!vis) return                                        // fora da tela: não gasta a GPU
    const w = Math.max(2, Math.round(r.width * Math.min(devicePixelRatio, 1.5)))
    const h = Math.max(2, Math.round(r.height * Math.min(devicePixelRatio, 1.5)))
    if (canvas.width !== w || canvas.height !== h) { canvas.width = w; canvas.height = h }
    gl.viewport(0, 0, canvas.width, canvas.height)
    gl.uniform2f(uRes, canvas.width, canvas.height)
    gl.uniform1f(uTime, faseDoMotor())     // a fase que o DTC (WASM) computou — o painel entrega pro canvas
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4)
  }
  registra(render)                          // entra no relógio único — a fase já está pronta quando roda
  render()                                  // o PRIMEIRO quadro JÁ: o canvas não entra preto à espera do rAF
  // hook de depuração: força um quadro fora do rAF (útil quando a aba está em background e o rAF pausa)
  const forcar = (tempo) => { const w = Math.max(2, Math.round((canvas.getBoundingClientRect().width || 480)))
    canvas.width = w; canvas.height = w; gl.viewport(0, 0, w, w); gl.uniform2f(uRes, w, w)
    gl.uniform1f(uTime, tempo ?? faseDoMotor()); gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4) }
  ;(window.__campoRenders = window.__campoRenders || []).push(forcar)
  return true
}
