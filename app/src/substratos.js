// Os substratos, ao vivo no navegador: o filtro do colapso em WASM (CPU) e GLSL (WebGL2, GPU).
// O predicado dep = (rel≠0) & existe — o MESMO que corre em wasmtime, moderngl, LLVM e PTX.

// ── WASM: a VM de pilha (o mesmo .wasm compilado pelo grafo) ──
export async function runWasm(url, arestas) {
  const buf = await (await fetch(url)).arrayBuffer()
  const { instance } = await WebAssembly.instantiate(buf)
  const n = arestas.length
  const i32 = new Int32Array(instance.exports.mem.buffer)
  for (let i = 0; i < n; i++) { i32[i] = arestas[i].rc; i32[n + i] = arestas[i].ex } // rel em [0,n), ex em [n,2n)
  instance.exports.filtrar(n)
  return Array.from({ length: n }, (_, i) => i32[2 * n + i])                          // dep em [2n,3n)
}

// ── GLSL: WebGL2, uma coluna por aresta (o Φ paralelo da GPU do navegador) ──
export function runGlsl(vsSrc, fsSrc, arestas) {
  const n = arestas.length
  const canvas = document.createElement('canvas')
  canvas.width = n; canvas.height = 1
  const gl = canvas.getContext('webgl2', { preserveDrawingBuffer: true })
  if (!gl) throw new Error('WebGL2 indisponível')
  const compile = (type, src) => {
    const s = gl.createShader(type); gl.shaderSource(s, src); gl.compileShader(s)
    if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(s))
    return s
  }
  const prog = gl.createProgram()
  gl.attachShader(prog, compile(gl.VERTEX_SHADER, vsSrc))
  gl.attachShader(prog, compile(gl.FRAGMENT_SHADER, fsSrc))
  gl.linkProgram(prog)
  if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) throw new Error(gl.getProgramInfoLog(prog))
  gl.useProgram(prog)
  const quad = gl.createBuffer(); gl.bindBuffer(gl.ARRAY_BUFFER, quad)
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-1, -1, 1, -1, -1, 1, 1, 1]), gl.STATIC_DRAW)
  const loc = gl.getAttribLocation(prog, 'pos')
  gl.enableVertexAttribArray(loc); gl.vertexAttribPointer(loc, 2, gl.FLOAT, false, 0, 0)
  const px = new Uint8Array(n * 4)
  for (let i = 0; i < n; i++) { px[4 * i] = arestas[i].rc; px[4 * i + 1] = arestas[i].ex; px[4 * i + 3] = 255 }
  const tex = gl.createTexture(); gl.bindTexture(gl.TEXTURE_2D, tex)
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, n, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, px)
  gl.uniform1i(gl.getUniformLocation(prog, 'dados'), 0)
  gl.viewport(0, 0, n, 1)
  gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4)
  const out = new Uint8Array(n * 4)
  gl.readPixels(0, 0, n, 1, gl.RGBA, gl.UNSIGNED_BYTE, out)
  return Array.from({ length: n }, (_, i) => (out[4 * i] > 127 ? 1 : 0))
}

const esc = (s) => String(s).replace(/[&<>]/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;' }[c]))

function badge(txt, ok) {
  return `<span class="sbadge${ok ? ' ok' : (ok === false ? ' err' : '')}"><span class="sdot"></span>${esc(txt)}</span>`
}

export function initSubstratos(data) {
  const sec = document.createElement('section')
  sec.className = 'blk'; sec.id = 'substratos'
  sec.innerHTML = `
    <div class="wrap">
      <h2>Os substratos <span class="sub">· o filtro do colapso, ao vivo no navegador</span></h2>
      <div class="rule"></div>
      <p class="sdesc">Nem tudo aqui é imagem: esta operação <b>roda agora, nesta página</b>. O
        <b>filtro das refs</b> — decidir se uma aresta do grafo é uma dependência a citar
        (<code>${esc(data.predicado)}</code>) — desce por todos os substratos de execução, cada um no seu Φ.
        Dois deles moram na Web: <b>WASM</b> (a VM de pilha, na CPU) e <b>GLSL</b> (WebGL2, uma coluna por aresta,
        na GPU). Mesmos dados, mesmo predicado — e o selo confirma que concordam.</p>
      <div class="console" id="sub-console">
        <div class="sbar">
          <span class="scall"><b>WebAssembly.instantiate</b>(${data.wasm_bytes} bytes) · <b>WebGL2</b> · ${data.arestas.length} de ${data.n_total} arestas reais</span>
          <span class="sspacer"></span>
          <span id="b-wasm">${badge('WASM …', null)}</span>
          <span id="b-glsl">${badge('GLSL …', null)}</span>
          <button id="sub-run" class="btnrun" type="button">↻ rodar de novo</button>
        </div>
        <div class="sscroll"><table class="stable">
          <thead><tr>
            <th>nó (de)</th><th>rel</th><th>alvo</th>
            <th class="c">rc≠0</th><th class="c">existe</th><th class="c">WASM</th><th class="c">GLSL</th>
          </tr></thead>
          <tbody id="sub-rows"></tbody>
        </table></div>
      </div>
      <p class="snote">O <code>.wasm</code> (161 bytes) e o <em>fragment shader</em> foram emitidos por um nó do
        grafo — <code>app_substratos.py</code> — que <b>certifica a costura</b> no metal (WASM em wasmtime, GLSL em
        moderngl, ambos ≡ o oráculo) antes de descer à Web. Aqui, o navegador a re-encena: <b>WASM ≡ GLSL ≡ o
        esperado</b>. As mesmas 4 linguagens ainda correm fora do navegador — LLVM (CPU), PTX (CUDA).</p>
    </div>`
  const rows = sec.querySelector('#sub-rows')
  const run = () => {
    Promise.resolve().then(async () => {
      let depW = null, depG = null, errW = '', errG = ''
      try { depW = await runWasm(data.wasm, data.arestas) } catch (e) { errW = e.message }
      try { depG = runGlsl(data.glsl.vertex, data.glsl.fragment, data.arestas) } catch (e) { errG = e.message }
      const esp = data.arestas.map((a) => a.esperado)
      const okW = depW && depW.every((d, i) => d === esp[i])
      const okG = depG && depG.every((d, i) => d === esp[i])
      sec.querySelector('#b-wasm').innerHTML = badge(errW ? 'WASM indisponível' : `WASM ${okW ? '≡ esperado' : 'divergiu'}`, errW ? false : okW)
      sec.querySelector('#b-glsl').innerHTML = badge(errG ? 'GLSL indisponível' : `GLSL ${okG ? '≡ esperado' : 'divergiu'}`, errG ? false : okG)
      rows.innerHTML = ''
      data.arestas.forEach((a, i) => {
        const dW = depW ? depW[i] : '—', dG = depG ? depG[i] : '—'
        const cell = (v) => v === 1 ? '<span class="stag pass">→ cita</span>' : (v === 0 ? '<span class="stag drop">descarta</span>' : '—')
        const tr = document.createElement('tr')
        tr.style.animationDelay = (i * 14) + 'ms'
        tr.innerHTML =
          `<td class="snode"><b>${esc(a.de)}</b></td><td class="srel">${esc(a.rel)}</td><td class="snode">${esc(a.alvo)}</td>` +
          `<td class="sbit ${a.rc ? 'on' : ''}">${a.rc ? 1 : 0}</td><td class="sbit ${a.ex ? 'on' : ''}">${a.ex}</td>` +
          `<td class="c">${cell(dW)}</td><td class="c">${cell(dG)}</td>`
        rows.appendChild(tr)
      })
    })
  }
  sec.querySelector('#sub-run').addEventListener('click', run)
  requestAnimationFrame(run)
  return sec
}
