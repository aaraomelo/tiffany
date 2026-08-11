import { resolveNoCorpo, tipoDe, FICHEIROS } from './src/corpo.js'
import { defineConfig } from 'vite'
import { fileURLToPath, URL } from 'node:url'
import { execSync } from 'node:child_process'
import { statSync, existsSync, readFileSync, copyFileSync, mkdirSync } from 'node:fs'
import { resolve, dirname } from 'node:path'

// A data da última atualização do enredo: sai do commit do FONTE (.tex), não da hora do build —
// rebuildar sem mexer no livro não deve mudar a data. Se não houver git, cai no mtime do arquivo.
// O enredo vive AQUI desde a consolidação (era chess/sandbox/reino_dourado_enredo.tex).
const fonteEnredo = fileURLToPath(new URL('../enredo.tex', import.meta.url))
function dataEnredo() {
  try {
    const iso = execSync(`git log -1 --format=%cI -- "${fonteEnredo}"`, { encoding: 'utf8' }).trim()
    if (iso) return iso
  } catch (e) { /* sem git: usa o arquivo */ }
  return statSync(fonteEnredo).mtime.toISOString()
}

// publicDir aponta para ../assets/figuras: o dev server serve /reino/*.png e /docs/*.pdf
// direto dos assets rasterizados na GPU (sem cópia, sem base64). base relativa p/ o build.
// (a pasta era ../figuras até 03/08, quando passou para dentro de assets/ — esta linha é a
//  ÚNICA referência em código a esse caminho; as outras eram documentação.)
// ── COMPOR AO CLICAR: /docs/*.pdf sai do .tex NA HORA, e nada fica pré-gravado ──────────
//
// O Aarão: «quando clica renderiza em tempo real, nada pré-gravado» · «lê no local dele e
// converte pra pdf».
//
// Um PDF pré-compilado é uma GRAVAÇÃO — a mesma coisa que o GIF era ao lado do kernel, e
// envelhece do mesmo modo: esta semana dois artigos foram servidos de uma publicação anterior
// enquanto o link devolvia 200. Aqui o pedido chega, o tradutor corre sobre o .tex ONDE ELE
// ESTÁ, e o PDF sai. Não se move nenhum ficheiro de lugar, e não fica cópia nenhuma no dist.
//
// O tradutor é o tests/tex.c — o corpo LaTeX, MOVE(latex, sentido) — e a largura do glifo vem
// da CURVA lida da TTF (lib/spline.h), não de uma tabela.
const DOCS = {
  'teoria':        '../teoria.tex',
  'catalogo':      '../catalogo.tex',
  'enredo':        '../enredo.tex',
  'livro':         '../livro.tex',
  'corpo-estelar': '../papers/corpo-estelar.tex',
  'dualsort':      '../papers/dualsort.tex',
  'fisica':        '../papers/fisica.tex',
  'medida':        '../papers/medida.tex',
  'milenio':       '../papers/milenio.tex',
  'arquitetura':   '../papers/arquitetura.tex',
}

function erro (res, msg) {
  res.statusCode = 500
  res.setHeader('Content-Type', 'text/plain; charset=utf-8')
  res.end(msg + '\n')
}

function compoeAoClicar() {
  const raiz = fileURLToPath(new URL('.', import.meta.url))
  const tradutor = resolve(raiz, '../tests/tex')
  return {
    name: 'compoe-ao-clicar',
    configureServer (server) { server.middlewares.use(mw) },
    configurePreviewServer (server) { server.middlewares.use(mw) },
  }
  function mw (req, res, next) {
    const m = /^\/docs\/([A-Za-z0-9_-]+)\.pdf(?:\?.*)?$/.exec(req.url || '')
    if (!m) return next()
    const fonte = DOCS[m[1]]
    if (!fonte) return next()                       // não é nosso: segue o caminho normal
    const tex = resolve(raiz, fonte)
    // E AQUI NÃO HÁ next(): eu tinha escrito `if (!existsSync(tradutor)) return next()`, que é
    // EXACTAMENTE o fallback silencioso que isto vem desfazer — sem o tradutor, o pedido caía no
    // publicDir e servia o PDF antigo com um 200 tranquilo. O medidor apanhou-o. Se o documento
    // é nosso, ou se compõe agora ou se diz que não se compôs.
    if (!existsSync(tex))
      return erro(res, `o fonte ${fonte} não está no lugar dele — e não há PDF a servir no lugar.`)
    if (!existsSync(tradutor))
      return erro(res, `o tradutor tests/tex não está compilado — e servir o PDF antigo seria\n` +
                       `voltar a gravar. (cc -O2 -std=c99 -I../lib tex.c -lm -o tex)`)
    // o destino é temporário e anónimo: o que se serve é o resultado, não um ficheiro guardado
    const saida = resolve(raiz, `../tests/.compoe_${m[1]}.pdf`)
    try {
      const t0 = Date.now()
      execSync(`${JSON.stringify(tradutor)} ${JSON.stringify(tex)} ${JSON.stringify(saida)}`,
               { stdio: 'pipe', cwd: dirname(tradutor), timeout: 120000 })
      const pdf = readFileSync(saida)
      res.setHeader('Content-Type', 'application/pdf')
      res.setHeader('Content-Length', pdf.length)
      // NÃO se guarda em cache: o ponto é sair na hora. Um cache aqui reintroduzia a gravação.
      res.setHeader('Cache-Control', 'no-store')
      res.setHeader('X-Composto-Em-Ms', String(Date.now() - t0))
      res.end(pdf)
    } catch (e) {
      // e falha RUIDOSAMENTE: um 500 diz que não compôs. Cair no ficheiro antigo seria servir
      // a gravação outra vez, calado — que é exactamente o defeito que isto vem desfazer.
      res.statusCode = 500
      res.setHeader('Content-Type', 'text/plain; charset=utf-8')
      res.end(`o tradutor não compôs ${m[1]}.tex — e não há PDF antigo a servir no lugar dele.\n` +
              String(e.stderr || e.message || e).slice(0, 400))
    }
  }
}

// ── O CORPO NO FRONT: /corpo/<caminho> serve o que o tradutor precisa, de onde ele está ──
//
// O Aarão: «põe os arquivos que vc precisa no front». A lista está medida (tools/corpo.sh, o
// fopen interceptado) e o portão é ela: sai o que lá está, e mais nada. Uma rota estática
// sobre a raiz servia o `curriculo/` — que tem CPF — com um 200 tranquilo.
function serveOCorpo () {
  const raiz = fileURLToPath(new URL('..', import.meta.url))
  return {
    name: 'serve-o-corpo',
    configureServer (server) { server.middlewares.use(mw) },
    configurePreviewServer (server) { server.middlewares.use(mw) },
    // E NO BUILD os mesmos ficheiros entram em dist/corpo/. Aqui há cópia, e há de propósito:
    // o nginx serve um DIRECTÓRIO, e um ficheiro que não está debaixo dele não existe para o
    // navegador. Mas não é uma gravação — uma gravação é um DERIVADO servido no lugar do que
    // devia ser composto; isto é a FONTE, e é dela que o PDF há-de sair no cliente.
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
    },
  }
  function mw (req, res, next) {
    if (!(req.url || '').startsWith('/corpo/')) return next()
    const rel = resolveNoCorpo(req.url)
    // e AQUI NÃO HÁ next(): um pedido a /corpo/ que não está na lista não deve cair no
    // publicDir e sair por outra porta. Ou está no manifesto, ou é recusado e diz-se porquê.
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
  plugins: [compoeAoClicar(), serveOCorpo()],
  base: './',
  publicDir: fileURLToPath(new URL('../assets/figuras', import.meta.url)),
  server: { open: true },
  define: { __ENREDO_ATUALIZADO__: JSON.stringify(dataEnredo()) },
})
