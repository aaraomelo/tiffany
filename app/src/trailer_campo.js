// ── O TRAILER, NO CIRCUITO — a cena em o campo (tempo real), dirigida pelo RELÓGIO-MESTRE ──
// O trailer tinha 72 quadros gravados (galáxia + orbes + garrafa + poinsétia empilhados): poluído, e FORA do
// circuito (não seguia o relógio). Agora é a cena SIMPLES do reino, no motor: os dois corações duais (o
// Príncipe ⋈ o Dark Pontryagin) e o fio do REI que os costura — a mesma cena do hero, mas percorrendo os 4
// LANCES do enredo pela FASE (o mesmo faseDoMotor, bit a bit): o coração, a fratura, a batalha, o mate.
// Nada empilhado: um raymarch, uma luz, um relógio. E o slider de velocidade o comanda (está no circuito).
import { faseDoMotor, kochTorre } from './motor_wasm.js'
import { registra } from './relogio.js'

const VS = `#version 300 es
in vec2 pos; void main(){ gl_Position = vec4(pos, 0.0, 1.0); }`

// os 4 LANCES (o enredo) — o ato é a fase: [0,¼) o coração · [¼,½) a fratura · [½,¾) a batalha · [¾,1) o mate
const FS = `#version 300 es
precision highp float;
uniform vec2 u_res; uniform float u_time;
uniform float u_kochAmp[6]; uniform float u_kochFreq[6];
out vec4 o;
const float TAU = 6.283185307179586;
const float C225 = 2.25, C200 = 0.045;
const float PHI = 1.618033988749895;                 // a defasagem áurea (o gap que move)
const vec3 OURO = vec3(1.00, 0.80, 0.32);            // o Príncipe (a luz, a exp)
const vec3 DARK = vec3(0.34, 0.26, 0.60);            // o Dark Pontryagin (a sombra, o dual)
const vec3 OURO_REI = vec3(1.00, 0.82, 0.30);        // o Rei (o relógio, o gap de fora)
const float RREI = 0.05;
float Feq(vec3 p){ float A=p.x*p.x+C225*p.y*p.y+p.z*p.z-1.0; float z3=p.z*p.z*p.z;
  return A*A*A - p.x*p.x*z3 - C200*p.y*p.y*z3; }
float resf(vec3 d){ return 1.0/sqrt(d.x*d.x+C225*d.y*d.y+d.z*d.z); }
float dFdr(vec3 d, float r){ vec3 q=r*d; float A=q.x*q.x+C225*q.y*q.y+q.z*q.z-1.0, A2=A*A, z2=q.z*q.z, z3=z2*q.z;
  float gx=6.0*q.x*A2-2.0*q.x*z3, gy=13.5*q.y*A2-2.0*C200*q.y*z3, gz=6.0*q.z*A2-3.0*q.x*q.x*z2-3.0*C200*q.y*q.y*z2;
  return gx*d.x+gy*d.y+gz*d.z; }
float rc(vec3 d){ float lo=0.15, hi=1.7;
  for(int i=0;i<16;i++){ float m=0.5*(lo+hi); if(Feq(m*d)<0.0) lo=m; else hi=m; }
  float r=0.5*(lo+hi);
  for(int i=0;i<3;i++){ r -= Feq(r*d)/dFdr(d,r); }   // Newton: a raiz exata da costura
  return r; }
float mapaUm(vec3 p, float a){ float rho=length(p); if(rho<1e-4) return -0.5;
  vec3 d=p/rho; return rho - mix(resf(d), rc(d), a); }
// a SEPARAÇÃO é o enredo: os dois corações se afastam (a FRATURA) e voltam (o MATE que não mata)
float sdRei(vec3 p, float dx){ vec3 a=vec3(-dx,0.0,0.0), b=vec3(dx,0.0,0.0), pa=p-a, ba=b-a;
  float h=clamp(dot(pa,ba)/dot(ba,ba),0.0,1.0); return length(pa-ba*h)-RREI; }
float mapaB(vec3 p, float aB, float dx){ vec3 q=p-vec3(dx,0.0,0.0); q.z=-q.z; return mapaUm(q,aB); }
float mapa2(vec3 p, float aA, float aB, float dx){
  return min(min(mapaUm(p-vec3(-dx,0.0,0.0), aA), mapaB(p,aB,dx)), sdRei(p,dx))*0.6; }
vec3 nrm2(vec3 p, float aA, float aB, float dx){ const vec2 k=vec2(1.0,-1.0); const float e=1.8e-3;
  return normalize(k.xyy*mapa2(p+k.xyy*e,aA,aB,dx) + k.yyx*mapa2(p+k.yyx*e,aA,aB,dx)
                 + k.yxy*mapa2(p+k.yxy*e,aA,aB,dx) + k.xxx*mapa2(p+k.xxx*e,aA,aB,dx)); }
float pulso(float off){ float ang=TAU*u_time+off; float harm=0.0;
  for(int j=0;j<6;j++) harm += u_kochAmp[j]*cos(u_kochFreq[j]*ang);
  return clamp(0.5-0.5*cos(ang)+0.03*harm, 0.0, 1.0); }
vec3 ceu(vec3 rd){ float t=clamp(rd.z*0.5+0.5,0.0,1.0);
  return mix(vec3(0.04,0.04,0.08), vec3(0.09,0.13,0.28), t) + exp2(-abs(rd.z)*5.0)*vec3(0.55,0.35,0.20); }
void main(){
  vec2 uv=(gl_FragCoord.xy*2.0 - u_res)/u_res.y;
  // o ENREDO na fase: um ato por volta ¼. A FRATURA afasta os corações; o MATE os traz de volta (não mata).
  float ato = fract(u_time);                          // [0,1) — a volta do relógio-mestre
  float sep = 0.5 - 0.5*cos(TAU*ato);                 // 0 (juntos) → 1 (fraturados) → 0 (o mate que não mata)
  float dx = mix(1.05, 2.10, sep);                    // a separação: a batalha é o pico do afastamento
  float aA = pulso(0.0);                              // o Príncipe
  float aB = pulso(PHI);                              // o Dark (a antifase áurea)
  vec3 ro=vec3(0.25, 5.9, 0.95), ta=vec3(0.0,0.0,-0.05);
  vec3 ww=normalize(ta-ro), uu=normalize(cross(ww,vec3(0.,0.,1.))), vv=cross(uu,ww);
  vec3 rd=normalize(uv.x*uu + uv.y*vv + 1.45*ww);
  float bb=dot(ro,rd), cc=dot(ro,ro)-12.25;           // bounding 3.5 (cobre a fratura máxima)
  float disc=bb*bb-cc;
  vec3 col;
  if(disc<=0.0){ col=ceu(rd); }
  else{
    float sd=sqrt(disc); float t=max(-bb-sd,0.0), tmax=-bb+sd;
    bool hit=false; vec3 p=ro;
    for(int i=0;i<64;i++){ p=ro+t*rd; float d=mapa2(p,aA,aB,dx); if(d<0.0012){hit=true;break;}
      t+=max(d*0.7,0.007); if(t>tmax)break; }
    if(hit){
      vec3 n=nrm2(p,aA,aB,dx); vec3 L=normalize(vec3(0.55,0.60,0.82)), v=-rd, h=normalize(L+v);
      float dif=max(dot(n,L),0.0), spec=pow(max(dot(n,h),0.0),40.0), fres=pow(1.0-max(dot(n,v),0.0),4.0);
      float dA=mapaUm(p-vec3(-dx,0.0,0.0),aA), dB=mapaB(p,aB,dx), dK=sdRei(p,dx);
      vec3 base; float emis=0.0;
      if(dK<dA && dK<dB){                             // o REI: o ponteiro (a fase) corre pela costura
        float hpos=clamp((p.x+dx)/(2.0*dx),0.0,1.0);
        emis=0.85*pow(0.5+0.5*cos(TAU*(hpos-u_time)),8.0); base=OURO_REI;
      } else base = dA<dB ? OURO : DARK;
      col = base*(0.34+0.82*dif) + spec*vec3(0.95) + fres*vec3(0.35,0.5,0.75)
          + 0.18*ceu(reflect(rd,n)) + emis*OURO_REI;
    } else col = ceu(rd);
  }
  o=vec4(pow(clamp(col,0.0,1.0), vec3(0.87)), 1.0);
}`

export function initTrailerCampo (host) {
  const canvas = document.createElement('canvas')
  const gl = canvas.getContext('webgl2', { antialias: true, alpha: false })
  if (!gl) return false                                    // sem WebGL2: fica o <img> (fallback)
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
  const t = kochTorre()                                    // a torre de Koch áurea (ℤ[φ]) — o torque fractal
  gl.uniform1fv(gl.getUniformLocation(prog, 'u_kochAmp'), new Float32Array(t.amps))
  gl.uniform1fv(gl.getUniformLocation(prog, 'u_kochFreq'), new Float32Array(t.freqs))

  const img = host.querySelector('img')                    // o trailer é o motor vivo
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
    gl.uniform1f(uTime, faseDoMotor())                     // a fase do relógio-mestre — o trailer NO circuito
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4)
  }
  registra(render)
  render()                                                 // o primeiro quadro JÁ (o canvas não entra preto)
  return true
}
