// banco_front.js — front Araano: fetch→wasm MOVE + traduz (manifesto).

import { abrirCanal, frontPedeRemoto, S_CANAL } from './canal_browser.js'
import { initBancoSql } from './banco_sql.js'
import { backendsDom, loadWasm, moveWasm } from './banco_move.js'
import { aresta, traduz, traduzWasm, validaArestas } from './banco_tradutor.js'
import { traduzCadeiaNode } from './c_wasm_shell.js'
import { manifestoAtual } from './manifesto_loader.js'
import { discoBrowser } from './banco_disco.js'

export async function bootBancoFront (opts = {}) {
  const wasmBase = opts.wasmBase || '/wasm/'
  const bancoBase = opts.bancoBase || '/banco/'
  const host = document.getElementById('banco')
  if (!host) throw new Error('#banco em falta')

  const disco = discoBrowser(opts)
  await initBancoSql({ ...opts, storage: disco })
  const man = manifestoAtual()
  const BACKENDS_DOM = backendsDom(man)

  let canal = null
  try {
    canal = abrirCanal()
    await canal.grava(S_CANAL, 0, 0)
  } catch {
    canal = null
  }

  const ctx = { wasmBase, bancoBase, sqlUrl: opts.sqlUrl, canal, storage: disco }
  const wasm = {}
  await Promise.all(BACKENDS_DOM.map(async (b) => {
    wasm[b.nome] = await loadWasm(wasmBase, b.wasm)
  }))
  const instancias = new Map(Object.entries(wasm))

  const errosArestas = validaArestas()
  if (errosArestas.length) console.warn('arestas:', errosArestas.join('; '))

  const { htmlSrc, cssSrc, jsSrc, modo, banda, nodeVer } = await carregaPagina(ctx)

  const html = moveWasm(wasm.html, 'html_move', htmlSrc, -1)
  const css = moveWasm(wasm.css, 'css_move', cssSrc, -1)
  const jsEsc = moveWasm(wasm.js, 'js_move', jsSrc, -1)
  const js = jsSrc

  const q = "INSERT TEXTO 'banco-front'"
  const qComp = moveWasm(wasm.sql, 'sql_move', q, -1, 100, 2000)
  const qRound = moveWasm(wasm.sql, 'sql_move', qComp, +1, 2000, 4000)
  if (qRound !== q) console.warn('sql_move† divergiu')

  const rotaCssHtml = aresta('css', 'html')
  const tradCssHtml = traduzWasm({ de: 'css', para: 'html', texto: cssSrc, instancias })

  montaDom(host, html, css, js)

  const meta = document.createElement('p')
  meta.className = 'banco-meta'
  meta.textContent =
    modo + ' · GKBANCO · traduz ' + rotaCssHtml.rota.join('→') + ' Q' + rotaCssHtml.a + rotaCssHtml.b +
    (tradCssHtml.texto ? ' ✓' : '') +
    (jsEsc !== js ? ' · js_escapar†' : '') +
    (nodeVer ? ' · ' + nodeVer : '') +
    (banda ? ' · banda ' + banda : '')
  Object.assign(meta.style, { font: '12px monospace', color: '#666', margin: '1rem', textAlign: 'center' })
  host.appendChild(meta)

  if (canal) canal.fechar()
}

function montaDom (host, html, css, js) {
  host.innerHTML = ''
  const root = document.createElement('div')
  root.className = 'banco-root'
  root.innerHTML = html
  const style = document.createElement('style')
  style.textContent = css
  host.appendChild(style)
  host.appendChild(root)
  const script = document.createElement('script')
  script.textContent = js
  host.appendChild(script)
}

function querRemoto () {
  const p = new URLSearchParams(location.search)
  if (p.get('remoto') === '1') return true
  if (p.get('remoto') === '0') return false
  return false
}

async function carregaPagina (ctx) {
  if (querRemoto() && ctx.canal) {
    try {
      const htmlSrc = await frontPedeRemoto(ctx.canal, 'html')
      const cssSrc = await frontPedeRemoto(ctx.canal, 'css')
      const jsSrc = await frontPedeRemoto(ctx.canal, 'js')
      if (htmlSrc && cssSrc && jsSrc) {
        return { htmlSrc, cssSrc, jsSrc, modo: 'canal→front', banda: ctx.canal.bandaHex() }
      }
    } catch (e) {
      console.warn('front remoto:', e.message || e)
    }
  }

  if (new URLSearchParams(location.search).get('metal') === '1') {
    try {
      return await carregaViaTradutor(ctx)
    } catch (e) {
      console.warn('metal:', e.message || e)
    }
  }

  const [htmlR, cssR, jsR] = await Promise.all([
    traduz({ de: 'fetch', para: 'html', texto: '', ctx }),
    traduz({ de: 'fetch', para: 'css', texto: '', ctx }),
    traduz({ de: 'fetch', para: 'js', texto: '', ctx }),
  ])
  return {
    htmlSrc: htmlR.texto,
    cssSrc: cssR.texto,
    jsSrc: jsR.texto,
    modo: 'fetch→traduz',
    banda: ctx.canal?.bandaHex?.() || null,
  }
}

async function carregaViaTradutor (ctx) {
  if (!ctx.canal) throw new Error('?metal=1 requer canal→node (Patria)')
  const script =
    "const { readFileSync } = require('node:fs');\n" +
    "const { join } = require('node:path');\n" +
    "const d=__dirname;\n" +
    "const payload={html:readFileSync(join(d,'pagina.html'),'utf8')," +
    "css:readFileSync(join(d,'pagina.css'),'utf8')," +
    "js:readFileSync(join(d,'pagina.js'),'utf8'),node:process.version,via:'canal→node'};\n" +
    'process.stdout.write(JSON.stringify(payload));\n'
  const raw = await traduzCadeiaNode(script, ctx)
  const data = parseJsonPagina(raw.texto)
  return {
    htmlSrc: data.html,
    cssSrc: data.css,
    jsSrc: data.js,
    modo: 'canal→node',
    nodeVer: data.node,
    banda: null,
  }
}

function parseJsonPagina (text) {
  const i = text.indexOf('{')
  const j = text.lastIndexOf('}')
  if (i < 0 || j <= i) throw new Error('JSON da pagina em falta')
  return JSON.parse(text.slice(i, j + 1))
}

bootBancoFront().catch((e) => {
  const host = document.getElementById('banco')
  if (host) {
    host.innerHTML = '<pre style="color:#c00;padding:1rem">' + String(e.message || e) + '</pre>'
  }
  console.error(e)
})
