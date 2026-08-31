// banco_front.js — front Araano: fetch→wasm MOVE + traduz (manifesto).
// estado remoto é eventual; DOM não depende dele. S_ESTADO fora do paint.
// primeiro paint não depende das capacidades que não estão a ser usadas.

import { abrirCanal, frontPedeRemoto, S_CANAL } from './canal_browser.js'
import { initBancoSql, shellMove } from './banco_sql.js'
import { backendsDom, loadWasm, moveWasm } from './banco_move.js'
import { aresta, traduz, traduzWasm, validaArestas } from './banco_tradutor.js'
import { manifestoAtual } from './manifesto_loader.js'
import { escolheDisco, leEstado, gravaEstado, gravaPagina, lePagina } from './banco_disco.js'
import { paginaParaU, uParaPagina } from './banco_pagina_u.js'
import { paramsDaSessao, urlCanal, sessaoParaU, modoDaSessao } from './banco_sessao_u.js'
import { discoIsolado, bordaDe, tenantParaU } from './banco_tenant_u.js'
import { urlHospedadoGk, ancoraDeHash, gravaAncora, leAncora, navParaU } from './banco_nav_u.js'
import { estadoGkDe, estadoGkParaU, VEL_OMISSAO } from './banco_estado_gk_u.js'
import { ligaIdentidade, identidadeParaU } from './banco_identidade_u.js'
import { integraCadeia, cadeiaParaU, donoDaCadeia } from './banco_cristalchain_u.js'
import { sincronizaEmFundo } from './banco_sync.js'
import { bandaDeTecido, hex16 } from './banda.js'

function aposQuadro () {
  return new Promise((r) => requestAnimationFrame(() => r()))
}

/** coord / selo / fuse / canal — depois do paint. */
function capacidadesPosPaint () {
  return Promise.all([
    import('./banco_coord_u.js'),
    import('./banco_coord_canal.js'),
  ]).then(([u, c]) => ({ ...u, ...c }))
}

function bootMarca (etapas, nome) {
  etapas.push({ nome, t: performance.now() })
}

function bootFecha (etapas) {
  const out = []
  for (let i = 1; i < etapas.length; i++) {
    out.push({ nome: etapas[i].nome, ms: Math.round(etapas[i].t - etapas[i - 1].t) })
  }
  const modulos = Math.round(etapas[0].t)
  const total = Math.round(etapas[etapas.length - 1].t - etapas[0].t)
  const atePaint = Math.round(etapas[etapas.length - 1].t)
  const recursos = (performance.getEntriesByType('resource') || []).map((e) => ({
    nome: String(e.name || '').replace(/^https:\/\/[^/]+/, ''),
    ms: Math.round(e.duration || 0),
    bytes: e.transferSize | 0,
    tipo: e.initiatorType || '',
  }))
  const boot = { atePaint, modulos, total, etapas: out, nRecursos: recursos.length, recursos }
  globalThis.__boot = boot
  console.info('boot', atePaint + 'ms (módulos ' + modulos + ' + boot ' + total + ')',
    out.map((e) => e.nome + '=' + e.ms).join(' '))
  return boot
}

export async function bootBancoFront (opts = {}) {
  const etapas = []
  bootMarca(etapas, 'start')
  const wasmBase = opts.wasmBase || '/wasm/'
  const bancoBase = opts.bancoBase || '/banco/'
  const host = document.getElementById('banco')
  if (!host) throw new Error('#banco em falta')

  const disco = await escolheDisco(opts)
  bootMarca(etapas, 'disco')

  const sess = paramsDaSessao()
  const discoId = discoIsolado(disco, sess.tenant)
  tenantParaU(bordaDe(
    typeof location !== 'undefined' ? location.host : '',
    typeof location !== 'undefined' ? location.pathname : '/banco/',
  ))
  const modoSessao = modoDaSessao(sess)
  let canalP = null
  if (modoSessao === 'remoto') {
    canalP = Promise.resolve().then(async () => {
      const c = abrirCanal({
        url: urlCanal(sess.endereco) || undefined,
        pub: sess.chave || undefined,
      })
      await c.grava(S_CANAL, 0, 0)
      return c
    }).catch(() => null)
  }

  await initBancoSql({ ...opts, storage: disco })
  bootMarca(etapas, 'sql+manifesto+celula')
  const man = manifestoAtual()
  const BACKENDS_DOM = backendsDom(man)

  const identidade = await ligaIdentidade(discoId, { chave: sess.chave })
  identidadeParaU(identidade, { modo: modoSessao, endereco: sess.endereco || '' })
  const cadeia = await integraCadeia(identidade, discoId)
  cadeiaParaU(cadeia)
  bootMarca(etapas, 'identidade+cadeia')

  const ctx = { wasmBase, bancoBase, sqlUrl: opts.sqlUrl, canal: null, storage: disco }
  const wasm = {}
  await Promise.all(BACKENDS_DOM.filter((b) => b.nome !== 'sql').map(async (b) => {
    wasm[b.nome] = await loadWasm(wasmBase, b.wasm, disco)
  }))
  const sqlL = BACKENDS_DOM.find((b) => b.nome === 'sql')
  if (sqlL) wasm.sql = await loadWasm(wasmBase, sqlL.wasm, disco)
  bootMarca(etapas, 'wasm-html-css-js-sql')
  const instancias = new Map(Object.entries(wasm))

  const errosArestas = validaArestas()
  if (errosArestas.length) console.warn('arestas:', errosArestas.join('; '))

  const { htmlSrc, cssSrc, jsSrc, modo, banda, nodeVer } = await carregaPagina(ctx)
  bootMarca(etapas, 'pagina-fetch')
  const nod = paginaParaU({ html: htmlSrc, css: cssSrc, js: jsSrc }, man)
  const P = uParaPagina(nod)
  if (P.html !== htmlSrc || P.css !== cssSrc || P.js !== jsSrc) {
    console.warn('pagina↔U: projecção perdeu texto')
  }

  const html = moveWasm(wasm.html, 'html_move', P.html, -1)
  const css = moveWasm(wasm.css, 'css_move', P.css, -1)
  const jsEsc = moveWasm(wasm.js, 'js_move', P.js, -1)
  const js = P.js

  const q = "INSERT TEXTO 'banco-front'"
  const qComp = moveWasm(wasm.sql, 'sql_move', q, -1, 100, 2000)
  const qRound = moveWasm(wasm.sql, 'sql_move', qComp, +1, 2000, 4000)
  if (qRound !== q) console.warn('sql_move† divergiu')

  const rotaCssHtml = aresta('css', 'html')
  const tradCssHtml = traduzWasm({ de: 'css', para: 'html', texto: P.css, instancias })
  bootMarca(etapas, 'move+traduz')

  montaDom(host, html, css, js)
  bootMarca(etapas, 'paint')
  bootFecha(etapas)

  await aposQuadro()
  hospedaGk(host, { tenant: sess.tenant, storage: discoId })
  let canal = null
  if (canalP) {
    canal = await canalP
    ctx.canal = canal
    bootMarca(etapas, canal ? 'wss-abre' : 'canal-falhou')
  }
  ligaShell(host, { canal, modo: modoSessao, storage: disco, sess })
  const cap = await capacidadesPosPaint()
  ligaContrato(host, { canal, storage: discoId, sess, cap })
  sessaoParaU(sess, man)
  // estado remoto é eventual; DOM não depende dele.
  const tSync = performance.now()
  sincronizaEmFundo(canal, disco).then((r) => {
    if (!globalThis.__boot) return
    globalThis.__boot.syncVia = r && r.via
    globalThis.__boot.syncMs = Math.round(performance.now() - tSync)
  })

  const ligacao = modoSessao === 'solo'
    ? 'M_wasm'
    : (canal ? 'M_wasm+M_remota' : 'M_wasm (canal sem resposta)')
  const meta = document.createElement('p')
  meta.className = 'banco-meta'
  meta.textContent =
    modo + ' · GKBANCO · ' + ligacao + ' · traduz ' + rotaCssHtml.rota.join('→') + ' Q' + rotaCssHtml.a + rotaCssHtml.b +
    (tradCssHtml.texto ? ' ✓' : '') +
    (jsEsc !== js ? ' · js_escapar†' : '') +
    (nodeVer ? ' · ' + nodeVer : '') +
    (banda ? ' · banda ' + banda : '') +
    (identidade.camada === 'chave' ? ' · id chave' : identidade.camada === 'sessao' ? ' · id sessao' : '') +
    (cadeia.livro && cadeia.livro.registos.length ? ' · cadeia ' + cadeia.livro.registos.length : '') +
    (sess.chave ? ' · pub' : '') +
    (sess.patria ? ' · patria' : '') +
    (sess.papel ? ' · ' + sess.papel + (sess.indice == null ? '' : ' i=' + sess.indice) : '')
  Object.assign(meta.style, { font: '12px monospace', color: '#666', margin: '1rem', textAlign: 'center' })
  host.appendChild(meta)
}

function ligaContrato (host, { canal, storage, sess, cap }) {
  const {
    abreJob, abreJobFuse, correFaixa, fechaContrato, leJob, gravaParte,
    parteFaixas, somaMonolito,
  } = cap
  const abre = host.querySelector('#bk-abre')
  const abreFuse = host.querySelector('#bk-abre-fuse')
  const corre = host.querySelector('#bk-corre')
  const fecha = host.querySelector('#bk-fecha')
  const out = host.querySelector('#bk-selo')
  const elCanal = host.querySelector('#bk-canal')
  if (!abre || !corre || !fecha || !out) return
  const JOB = 'web'
  const N = 10000
  const W = 4
  const pubs = ['00'.repeat(32), '11'.repeat(32), '22'.repeat(32), '33'.repeat(32)]
  const papel = (sess && sess.papel) || ''
  const diz = (t) => { out.textContent = t }
  async function bytesCanal () {
    try {
      if (canal && typeof canal.banda === 'function') {
        const b = await canal.banda()
        if (b && b.length) return b
      }
    } catch { /* solo: tecido */ }
    return bandaDeTecido()
  }
  function textoFecha (f) {
    const alvo = somaMonolito(N)
    return (f.fecha ? 'fechou' : 'não fechou') +
      ' — ' + (f.motivo || '') +
      '\njunta ' + f.junta + (f.junta === alvo ? ' = monolito' : '') +
      (f.sigma ? '\nσ ' + String(f.sigma).slice(0, 16) : '') +
      (f.fecha ? '' : '\nfaltam partes/chaves ou forja')
  }
  if (papel === 'coord' || papel === 'worker') {
    ligaContratoWss(host, {
      canal, storage, sess, abre, abreFuse, corre, fecha, elCanal, diz, textoFecha,
      JOB, N, W, pubs, bytesCanal, cap,
    })
    return
  }
  let partes = []
  let lado = 'parte'
  abre.addEventListener('click', async () => {
    diz('…')
    try {
      lado = 'parte'
      const canalBytes = await bytesCanal()
      const aberto = await abreJob({
        storage, jobId: JOB, n: N, workers: W, canal: canalBytes,
      })
      partes = aberto.partes
      if (elCanal) elCanal.textContent = hex16(canalBytes)
      diz('partição — ' + W + ' pedaços; canal ' + hex16(canalBytes) + '\n' +
        '1 banda → N complementares; corre as N')
    } catch (e) {
      diz(String(e.message || e))
    }
  })
  if (abreFuse) {
    abreFuse.addEventListener('click', async () => {
      diz('…')
      try {
        lado = 'fuse'
        partes = []
        await abreJobFuse({ storage, jobId: JOB, n: N, workers: W })
        if (elCanal) elCanal.textContent = 'fuse'
        diz('fusão — ' + W + ' chaves independentes\n' +
          'cada K_i → id_i; corre as N; σ no fecho')
      } catch (e) {
        diz(String(e.message || e))
      }
    })
  }
  corre.addEventListener('click', async () => {
    const job = leJob(storage, JOB)
    if (!job) {
      diz('abre primeiro')
      return
    }
    if (lado === 'parte' && !partes.length) {
      diz('abre partição primeiro')
      return
    }
    diz('…')
    try {
      const faixas = parteFaixas(N, W)
      const linhas = []
      for (let i = 0; i < W; i++) {
        const id = await ligaIdentidade(storage, { chave: pubs[i] })
        const { dono, camada } = await donoDaCadeia(id)
        if (lado === 'parte') gravaParte(storage, JOB, i, partes[i])
        const r = correFaixa({
          storage, dono, camada, jobId: JOB, i,
          a: faixas[i].a, b: faixas[i].b, n: N, workers: W,
          parte: lado === 'parte' ? partes[i] : '',
          chave: lado === 'fuse' ? pubs[i] : '',
          idFuse: lado === 'fuse' ? id.id : '',
        })
        linhas.push('id' + i + ' liq=' + r.liq.liquidado + ' soma=' + (r.peca && r.peca.soma))
      }
      diz('correu as ' + W + ' (' + lado + ')\n' + linhas.join('\n'))
    } catch (e) {
      diz(String(e.message || e))
    }
  })
  fecha.addEventListener('click', async () => {
    diz('…')
    try {
      const f = await fechaContrato(leJob(storage, JOB))
      if (f.sigma && elCanal) elCanal.textContent = String(f.sigma).slice(0, 16)
      diz(textoFecha(f))
    } catch (e) {
      diz(String(e.message || e))
    }
  })
}

function ligaContratoWss (host, o) {
  const { canal, storage, sess, abre, abreFuse, corre, fecha, elCanal, diz, textoFecha, JOB, N, W, pubs, bytesCanal, cap } = o
  const {
    abreJob, abreJobFuse, correFaixa, fechaContrato, leJob, gravaJob,
    parteFaixas, enviaAbre, enviaFecho, enviaPeca, ouveCoord, ouvePeca,
  } = cap
  if (!canal) {
    diz('WSS — falta o canal (?patria=1 liga addr+pub)')
    return
  }
  const papel = sess.papel
  const indice = sess.indice == null ? 0 : (sess.indice | 0)
  const faixas = parteFaixas(N, W)
  const modoSess = sess.modo === 'fuse' ? 'fuse' : 'parte'

  if (papel === 'worker') {
    if (indice < 0 || indice >= W) {
      diz('worker i fora de 0..' + (W - 1))
      return
    }
    diz('worker ' + indice + ' — à espera do coord no WSS')
    ouveCoord(canal, (msg) => {
      if (msg.tipo === 'abre') {
        Promise.resolve().then(async () => {
          try {
            const fuse = msg.modo === 'fuse'
            gravaJob(storage, {
              id: msg.jobId || JOB,
              n: msg.n | 0,
              workers: msg.workers | 0,
              pecas: {},
              canal: String(msg.canal || ''),
              compromissos: Array.isArray(msg.compromissos) ? msg.compromissos : [],
              modo: fuse ? 'fuse' : 'parte',
            })
            const i = indice
            const chave = pubs[i]
            const parte = fuse ? '' : ((msg.partes && msg.partes[i]) || '')
            if (!fuse) gravaParte(storage, JOB, i, parte)
            if (elCanal) elCanal.textContent = fuse ? 'fuse i=' + i : String(msg.canal || '').slice(0, 16)
            const id = await ligaIdentidade(storage, { chave })
            const { dono, camada } = await donoDaCadeia(id)
            const fx = (msg.faixas && msg.faixas[i]) || faixas[i]
            const r = correFaixa({
              storage, dono, camada, jobId: JOB, i,
              a: fx.a, b: fx.b, n: msg.n | 0, workers: msg.workers | 0,
              parte, chave: fuse ? chave : '', idFuse: fuse ? id.id : '',
            })
            if (!r.peca) {
              diz('worker ' + i + ' — não liquidou')
              return
            }
            await enviaPeca(canal, i, {
              jobId: JOB, dono, a: fx.a, b: fx.b, soma: r.peca.soma,
              parte, chave: fuse ? chave : '', idFuse: fuse ? id.id : '',
              modo: fuse ? 'fuse' : 'parte',
            })
            diz('worker ' + i + ' enviou peca soma=' + r.peca.soma + (fuse ? ' fuse' : ''))
          } catch (e) {
            diz(String(e.message || e))
          }
        })
      }
      if (msg.tipo === 'fecho') {
        if (msg.sigma && elCanal) elCanal.textContent = String(msg.sigma).slice(0, 16)
        diz(textoFecha({ fecha: msg.fecha, junta: msg.junta, motivo: msg.motivo, sigma: msg.sigma }))
      }
    })
    abre.addEventListener('click', () => diz('este tab é worker — o coord abre'))
    if (abreFuse) abreFuse.addEventListener('click', () => diz('este tab é worker — o coord abre fusão'))
    corre.addEventListener('click', () => diz('este tab corre ao receber abre'))
    fecha.addEventListener('click', () => diz('este tab é worker — o coord fecha'))
    return
  }

  diz('coord — partição ou fusão no WSS; os 4 workers enviam peca')
  for (let i = 0; i < W; i++) {
    ouvePeca(canal, i, (msg) => {
      if (msg.tipo !== 'peca') return
      const job = leJob(storage, JOB)
      if (!job) return
      const idx = msg.i | 0
      if (!job.pecas[idx]) {
        job.pecas[idx] = {
          dono: String(msg.dono || ''),
          i: idx,
          a: msg.a | 0,
          b: msg.b | 0,
          soma: Number(msg.soma),
          parte: String(msg.parte || ''),
        }
        if (msg.chave) job.pecas[idx].chave = String(msg.chave)
        if (msg.idFuse) job.pecas[idx].idFuse = String(msg.idFuse)
        if (msg.modo === 'fuse') job.modo = 'fuse'
        gravaJob(storage, job)
      }
      const nP = Object.keys(job.pecas).length
      diz('coord pecas ' + nP + '/' + W)
      if (nP >= W) {
        Promise.resolve().then(async () => {
          try {
            const f = await fechaContrato(leJob(storage, JOB))
            if (f.sigma && elCanal) elCanal.textContent = String(f.sigma).slice(0, 16)
            diz(textoFecha(f))
            await enviaFecho(canal, {
              jobId: JOB, fecha: f.fecha, junta: f.junta, motivo: f.motivo, sigma: f.sigma || '',
            })
          } catch (e) {
            diz(String(e.message || e))
          }
        })
      }
    })
  }
  abre.addEventListener('click', async () => {
    diz('…')
    try {
      const canalBytes = await bytesCanal()
      const aberto = await abreJob({
        storage, jobId: JOB, n: N, workers: W, canal: canalBytes,
      })
      if (elCanal) elCanal.textContent = hex16(canalBytes)
      await enviaAbre(canal, {
        modo: 'parte',
        jobId: JOB,
        n: N,
        workers: W,
        canal: aberto.job.canal,
        compromissos: aberto.job.compromissos,
        faixas,
        partes: aberto.partes,
      })
      diz('partição no WSS — ' + W + ' pedaços; canal ' + hex16(canalBytes))
    } catch (e) {
      diz(String(e.message || e))
    }
  })
  if (abreFuse) {
    abreFuse.addEventListener('click', async () => {
      diz('…')
      try {
        await abreJobFuse({ storage, jobId: JOB, n: N, workers: W })
        if (elCanal) elCanal.textContent = 'fuse'
        await enviaAbre(canal, {
          modo: 'fuse',
          jobId: JOB,
          n: N,
          workers: W,
          faixas,
        })
        diz('fusão no WSS — N chaves independentes; workers i=0..3')
      } catch (e) {
        diz(String(e.message || e))
      }
    })
  }
  if (modoSess === 'fuse') {
    diz('coord fuse — abre fusão no WSS')
  }
  corre.addEventListener('click', () => diz('os workers correm; este tab junta'))
  fecha.addEventListener('click', async () => {
    diz('…')
    try {
      const f = await fechaContrato(leJob(storage, JOB))
      if (f.sigma && elCanal) elCanal.textContent = String(f.sigma).slice(0, 16)
      diz(textoFecha(f))
      await enviaFecho(canal, {
        jobId: JOB, fecha: f.fecha, junta: f.junta, motivo: f.motivo, sigma: f.sigma || '',
      })
    } catch (e) {
      diz(String(e.message || e))
    }
  })
}

function ligaShell (host, { canal, modo, storage, sess }) {
  const run = host.querySelector('#bk-run')
  const cmd = host.querySelector('#bk-cmd')
  const sh = host.querySelector('#bk-sh')
  const out = host.querySelector('#bk-out')
  if (!run || !cmd) return
  const remoto = modo === 'remoto' && !!canal
  if (out && !out.textContent) {
    out.textContent = remoto
      ? (sess && sess.patria
        ? 'Patria — WSS /canal; banda da chave pública'
        : 'canal — duas realizações (wasm + remota); disco sincroniza')
      : (modo === 'remoto'
        ? 'M_wasm — canal sem resposta; disco local'
        : 'solo — realização wasm; localStorage é o disco')
  }
  run.addEventListener('click', async () => {
    const backend = (sh && sh.value) || 'bash'
    if (out) out.textContent = '…'
    try {
      const txt = await shellMove(cmd.value, canal, backend, { remoto, storage })
      if (out) out.textContent = txt || '(vazio)'
    } catch (e) {
      if (out) out.textContent = String(e.message || e)
    }
  })
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

/** Golden Kingdom original em paralelo — depois do paint. Não importa main.js. */
function querGk () {
  return new URLSearchParams(location.search).get('gk') === '1'
}

function hospedaGk (host, opts = {}) {
  if (!querGk()) return
  const tenant = opts.tenant || ''
  const storage = opts.storage
  const ancora = ancoraDeHash(location.hash) || leAncora(storage)
  navParaU({ tenant, ancora })
  estadoGkParaU(estadoGkDe({ tenant, ancora, vel: VEL_OMISSAO }))
  const sec = document.createElement('section')
  sec.className = 'bk-shell'
  sec.id = 'bk-gk'
  const h = document.createElement('h2')
  h.textContent = 'Golden Kingdom'
  const p = document.createElement('p')
  p.className = 'bk-lede'
  p.textContent = tenant
    ? ('entrada no tenant ' + tenant + ' — navegação original (#id) no iframe.')
    : 'entrada do Reino — navegação original (#id) no iframe (oráculo).'
  const frame = document.createElement('iframe')
  frame.src = urlHospedadoGk({ ancora })
  frame.title = 'Reino Dourado'
  frame.dataset.tenant = tenant
  frame.setAttribute('loading', 'lazy')
  sec.appendChild(h)
  sec.appendChild(p)
  sec.appendChild(frame)
  host.appendChild(sec)
  ligaNavGk(frame, storage, tenant)
  /* F3: card é capacidade. Módulo pequeno, só depois do paint; kernels/GLSL/LaTeX ficam fora. */
  import('./banco_cards_u.js').catch(() => {})
}

function ligaNavGk (frame, storage, tenant) {
  let sync = false
  const aplica = (ancora, origem) => {
    const a = ancoraDeHash(ancora)
    if (sync) return
    sync = true
    try {
      gravaAncora(storage, a)
      navParaU({ tenant, ancora: a })
      const hash = a ? '#' + a : ''
      if (origem !== 'pai' && location.hash !== hash) {
        const u = location.pathname + location.search + hash
        history.replaceState(null, '', u)
      }
      if (origem !== 'iframe') {
        try {
          const w = frame.contentWindow
          if (w && w.location && (w.location.hash || '') !== hash) w.location.hash = a
        } catch { /* iframe ainda não same-origin */ }
      }
    } finally {
      sync = false
    }
  }
  window.addEventListener('hashchange', () => aplica(location.hash, 'pai'))
  frame.addEventListener('load', () => {
    try {
      const w = frame.contentWindow
      if (!w) return
      aplica(w.location.hash || ancoraDeHash(location.hash), 'iframe')
      w.addEventListener('hashchange', () => aplica(w.location.hash, 'iframe'))
    } catch { /* oráculo noutro origin */ }
  })
}

function querRemoto () {
  const p = new URLSearchParams(location.search)
  if (p.get('remoto') === '1') return true
  if (p.get('remoto') === '0') return false
  return false
}

async function carregaPagina (ctx) {
  const persist = (htmlSrc, cssSrc, jsSrc, modo, extra = {}) => {
    if (ctx.storage) {
      const e = leEstado(ctx.storage)
      gravaPagina(e, { html: htmlSrc, css: cssSrc, js: jsSrc })
      gravaEstado(e, ctx.storage)
    }
    return { htmlSrc, cssSrc, jsSrc, modo, ...extra }
  }
  const doLs = () => {
    const p = lePagina(leEstado(ctx.storage))
    if (!p) return null
    return { htmlSrc: p.html, cssSrc: p.css, jsSrc: p.js, modo: 'LS' }
  }

  if (querRemoto() && ctx.canal) {
    try {
      const htmlSrc = await frontPedeRemoto(ctx.canal, 'html')
      const cssSrc = await frontPedeRemoto(ctx.canal, 'css')
      const jsSrc = await frontPedeRemoto(ctx.canal, 'js')
      if (htmlSrc && cssSrc && jsSrc) {
        return persist(htmlSrc, cssSrc, jsSrc, 'canal→front', { banda: ctx.canal.bandaHex() })
      }
    } catch (e) {
      console.warn('front remoto:', e.message || e)
    }
  }

  /* ?metal=1: query load-bearing; pede a realização remota via canal→node. */
  if (new URLSearchParams(location.search).get('metal') === '1') {
    try {
      const r = await carregaViaTradutor(ctx)
      return persist(r.htmlSrc, r.cssSrc, r.jsSrc, r.modo, { nodeVer: r.nodeVer, banda: r.banda })
    } catch (e) {
      console.warn('realizacao remota:', e.message || e)
    }
  }

  try {
    const [htmlR, cssR, jsR] = await Promise.all([
      traduz({ de: 'fetch', para: 'html', texto: '', ctx }),
      traduz({ de: 'fetch', para: 'css', texto: '', ctx }),
      traduz({ de: 'fetch', para: 'js', texto: '', ctx }),
    ])
    return persist(htmlR.texto, cssR.texto, jsR.texto, 'fetch→traduz', {
      banda: ctx.canal?.bandaHex?.() || null,
    })
  } catch (e) {
    const ls = doLs()
    if (ls) return ls
    throw e
  }
}

async function carregaViaTradutor (ctx) {
  if (!ctx.canal) throw new Error('?metal=1 pede a realização remota (canal→node)')
  const script =
    "const { readFileSync } = require('node:fs');\n" +
    "const { join } = require('node:path');\n" +
    "const d=__dirname;\n" +
    "const payload={html:readFileSync(join(d,'pagina.html'),'utf8')," +
    "css:readFileSync(join(d,'pagina.css'),'utf8')," +
    "js:readFileSync(join(d,'pagina.js'),'utf8'),node:process.version,via:'canal→node'};\n" +
    'process.stdout.write(JSON.stringify(payload));\n'
  const { traduzCadeiaNode } = await import('./c_wasm_shell.js')
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
