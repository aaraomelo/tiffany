import { resolveNoCorpo, tipoDe, FICHEIROS } from './src/corpo.js'
import { defineConfig } from 'vite'
import { fileURLToPath, URL } from 'node:url'
import { execSync } from 'node:child_process'
import { statSync, existsSync, readFileSync, copyFileSync, mkdirSync } from 'node:fs'
import { resolve, dirname } from 'node:path'

// A data da última atualização do enredo: sai do commit do FONTE (.tex), não da hora do build —
// rebuildar sem mexer no livro não deve mudar a data. Se não houver git, cai no mtime do arquivo.
const fonteEnredo = fileURLToPath(new URL('../enredo.tex', import.meta.url))
function dataEnredo() {
  try {
    const iso = execSync(`git log -1 --format=%cI -- "${fonteEnredo}"`, { encoding: 'utf8' }).trim()
    if (iso) return iso
  } catch (e) { /* sem git: usa o arquivo */ }
  return statSync(fonteEnredo).mtime.toISOString()
}

function erro (res, msg) {
  res.statusCode = 500
  res.setHeader('Content-Type', 'text/plain; charset=utf-8')
  res.end(msg + '\n')
}

// ── O CORPO NO FRONT: /corpo/<caminho> serve o que o tradutor WASM precisa ──
//
// O Aarão: PDF no cliente via WASM, sem TeX Live, sem middleware Node a fingir
// de compositor. A lista está medida (tools/corpo.sh); o nginx/Vite só SERVEM
// a fonte — a composição é tex.wasm no browser.
function serveOCorpo () {
  const raiz = fileURLToPath(new URL('..', import.meta.url))
  return {
    name: 'serve-o-corpo',
    configureServer (server) { server.middlewares.use(mw) },
    configurePreviewServer (server) { server.middlewares.use(mw) },
    closeBundle () {
      const dest = resolve(raiz, 'app/dist/corpo')
      let n = 0
      for (const rel of FICHEIROS) {
        const de = resolve(raiz, rel)
        if (!existsSync(de)) throw new Error(`corpo: ${rel} está no manifesto e não no disco`)
        const para = resolve(dest, rel)
        mkdirSync(dirname(para), { recursive: true })
        copyFileSync(de, para)
        n++
      }
      console.log(`  corpo: ${n} ficheiros em dist/corpo/ (medidos por tools/corpo.sh)`)
      // tex.wasm vive em assets/figuras/wasm/ (publicDir → /wasm/tex.wasm). Confirma-se
      // no dist: sem ele o cliente não compõe nada.
      const wasm = resolve(raiz, 'app/dist/wasm/tex.wasm')
      if (!existsSync(wasm))
        throw new Error('dist/wasm/tex.wasm em falta — corre tools/sobe_tex_wasm.sh antes do build')
      console.log(`  wasm: tex.wasm (${statSync(wasm).size} bytes)`)
    },
  }
  function mw (req, res, next) {
    if (!(req.url || '').startsWith('/corpo/')) return next()
    const rel = resolveNoCorpo(req.url)
    if (!rel) {
      res.statusCode = 404
      res.setHeader('Content-Type', 'text/plain; charset=utf-8')
      return res.end('não está no corpo: só sai o que tools/corpo.sh mediu que o tradutor abre.\n')
    }
    const cam = resolve(raiz, rel)
    if (!existsSync(cam))
      return erro(res, `${rel} está no manifesto e não está no disco — remede com tools/corpo.sh.`)
    const b = readFileSync(cam)
    res.setHeader('Content-Type', tipoDe(rel))
    res.setHeader('Content-Length', b.length)
    res.end(b)
  }
}

export default defineConfig({
  plugins: [serveOCorpo()],
  base: './',
  // publicDir: /reino/*.png, /wasm/tex.wasm — o tradutor no cliente, não PDFs pré-gravados
  publicDir: fileURLToPath(new URL('../assets/figuras', import.meta.url)),
  server: { open: true },
  define: { __ENREDO_ATUALIZADO__: JSON.stringify(dataEnredo()) },
})
