import './style.css'
import manifesto from './manifesto.json'
import art from './kernels_campo.json'         // os kernels EMITIDOS pela broca (chessc) + as assinaturas do metal
import { initCardsKernel } from './cards_kernel.js'
import substratosData from './substratos.json'
import { initSubstratos } from './substratos.js'
import pontesData from './pontes.json'
import { initPontes } from './pontes.js'
import { initVelocidade } from './velocidade.js'
import { initMotorCampo } from './motor_campo.js'
import { initTrailerCampo } from './trailer_campo.js'
import { initCardsCampo } from './cards_campo.js'
import { initMotorWasm, avancaFase } from './motor_wasm.js'
import { iniciaRelogio } from './relogio.js'
import { abrirDoc, idDeArquivo } from './tex_tradutor.js'

// ── helpers ──
const el = (html) => { const t = document.createElement('template'); t.innerHTML = html.trim(); return t.content.firstChild }
const esc = (s) => String(s).replace(/[&<>"]/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]))
const tags = (arr) => arr.map((t) => `<span class="tag"><b>${esc(t.rot)}</b> ${esc(t.val)}</span>`).join('')

// ── os hosts que O MOTOR assume. O corpo é a FASE — o relógio, em ponto fixo, com o wrap a ser
//    um AND. Quem desenha é roupa; quem conta as voltas é o relógio.
// O <img> desses hosts é só o FALLBACK de quem não tem WebGL2 — e por isso nasce SEM src (o caminho fica no
// data-src). Antes ele nascia com src e loading=eager: o navegador baixava a gravação, pintava
// o modelo antigo, e o motor o descartava um segundo depois — via-se a esfera velha por baixo, e a banda ia fora.
// Quem dá o src de volta é `revelaFallback()`, e só onde o motor NÃO subiu.
// os dois que ainda não têm kernel emitido — ficam nomeados, e não escondidos
const MOTOR = new Set(['coracao_revela', 'captura'])
// não há ficheiro a servir: todo card é kernel, e os dois sem kernel emitido ficam nomeados acima

// ── as peças que O KERNEL rasteriza: o campo é computado a cada quadro ──
// (o nome não basta: o mesmo personagem existe em várias seções, e cada uma é outra arte — casa-se por seção)
// a CHAVE do card no artefato: composta (nome@seção) primeiro — o mesmo personagem existe em várias seções
// (o benjamim 2D e o do elenco3d são cards distintos); senão a simples (as seções 2D sem colisão).
const chaveKernel = (p, sec) => {
  if (art.pecas[p.nome + '@' + sec]) return p.nome + '@' + sec
  const a = art.pecas[p.nome]
  return (a && a.secao === sec) ? p.nome : null
}
const temKernel = (p, sec) => !!chaveKernel(p, sec)

// ── um card ──
function card(p, wide, sec) {
  // o card do MOTOR é RENDERIZADO em tempo real (o kernel computa o campo a cada quadro) — não há arquivo/gravação:
  // uma gravação não é uma renderização — e aqui não há gravação nenhuma.
  if (temKernel(p, sec)) {
    return `
    <article class="card${wide ? ' widec' : ''}">
      <div class="frame motor" data-peca="${esc(chaveKernel(p, sec))}" aria-label="${esc(p.titulo)}"></div>
      <div class="meta">
        <p class="nm">${esc(p.titulo)}</p>
        <p class="ep">${esc(p.ep)}</p>
        ${p.estrela ? `<span class="estrela e-${p.estrela === 'ESTRELA' ? 'e' : p.estrela === 'BURACO BRANCO' ? 'b' : 'n'}" title="${esc(p.estrela_porque || '')}">${p.estrela === 'ESTRELA' ? '\u2b50 estrela' : p.estrela === 'BURACO BRANCO' ? '\u25cb branco' : '\u25cf negro'}</span>` : ''}
        <span class="op">${esc(p.op)}</span>
        <p class="desc">${esc(p.desc)}</p>
        ${p.prosa ? `<p class="prosa">${esc(p.prosa)}</p>` : ''}
        <div class="tags">${tags(p.tags)}</div>
      </div>
    </article>`
  }
  // Nenhum card tem ficheiro: todos são kernel. Os que ainda não têm kernel emitido ficam com a
  // moldura vazia e NOMEADOS — melhor um lugar visível por preencher do que uma gravação a fingir.
  const blend = wide ? '' : ' blend'
  const mot = MOTOR.has(p.nome) ? ' motor' : ''
  return `
    <article class="card${wide ? ' widec' : ''}">
      <div class="frame${blend}${mot}" data-peca="${esc(p.nome)}" aria-label="${esc(p.titulo)}"></div>
      <div class="meta">
        <p class="nm">${esc(p.titulo)}</p>
        <p class="ep">${esc(p.ep)}</p>
        ${p.estrela ? `<span class="estrela e-${p.estrela === 'ESTRELA' ? 'e' : p.estrela === 'BURACO BRANCO' ? 'b' : 'n'}" title="${esc(p.estrela_porque || '')}">${p.estrela === 'ESTRELA' ? '\u2b50 estrela' : p.estrela === 'BURACO BRANCO' ? '\u25cb branco' : '\u25cf negro'}</span>` : ''}
        <span class="op">${esc(p.op)}</span>
        <p class="desc">${esc(p.desc)}</p>
        ${p.prosa ? `<p class="prosa">${esc(p.prosa)}</p>` : ''}
        <div class="tags">${tags(p.tags)}</div>
      </div>
    </article>`
}

function secao(s) {
  const wide = !!s.wide
  const solo = s.pecas.length === 1   // uma só peça: o card preenche (imagem grande + texto ao lado), responsivo
  return `
    <section class="blk" id="${s.id}">
      <div class="wrap">
        <h2>${esc(s.titulo)} <span class="sub">· ${esc(s.sub)}</span></h2>
        <div class="rule"></div>
        <div class="grid${wide ? ' wide' : ''}${solo ? ' solo' : ''}">
          ${s.pecas.map((p) => card(p, wide, s.id)).join('')}
        </div>
      </div>
    </section>`
}

function trailer(tr) {
  const lances = tr.lances.map((l) => `
    <div class="lance"><div class="n">${esc(l.n)}</div>
      <div><h3>${esc(l.titulo)}</h3><p>${l.texto}</p></div></div>`).join('')
  return `
    <section class="trailer" id="trailer">
      <div class="wrap" style="max-width:940px">
        <h2 style="font-family:Georgia,serif;font-size:clamp(1.8rem,4vw,2.6rem);margin:0;color:var(--ink)">
          ${esc(tr.titulo)} <span style="color:var(--ink3);font-size:.5em">· o enredo em movimento</span></h2>
        <div class="screen motor" data-peca="trailer" aria-label="${esc(tr.titulo)} — o trailer"></div>
        <p class="ep" style="color:var(--ink3);text-align:center;margin-top:12px">
          Em <b>tempo real</b>, no circuito do reino (o mesmo relógio-mestre, bit a bit — resíduo 0) · os dois corações
          duais (o Príncipe ⋈ o Dark Pontryagin, em antifase áurea) e o fio do <b>Rei</b> que os costura · a fase
          percorre os quatro lances: o coração, a fratura, a batalha, o mate que não mata</p>
        <div class="lances">${lances}</div>
      </div>
    </section>`
}

// a data da última atualização do enredo, injetada no build (ver vite.config.js)
const ATUALIZADO = typeof __ENREDO_ATUALIZADO__ === 'string' ? __ENREDO_ATUALIZADO__ : ''
function dataCurta(iso) {
  if (!iso) return ''
  const d = new Date(iso)
  if (isNaN(d)) return ''
  return d.toLocaleDateString('pt-BR', { day: '2-digit', month: '2-digit', year: 'numeric' })
}
const ATUALIZADO_BR = dataCurta(ATUALIZADO)

function docs(dd) {
  const ic = { 'enredo.pdf': '📜', 'teoria.pdf': '📐', 'catalogo.pdf': '📖', 'livro.pdf': '📚',
               'corpo-estelar.pdf': '⭐', 'dualsort.pdf': '🔀', 'fisica.pdf': '⚛️',
               'medida.pdf': '📏', 'milenio.pdf': '∞', 'arquitetura.pdf': '🏗️',
               'corpo-peano.pdf': '🎵' }
  const items = dd.map((d) => {
    const key = d.arquivo.split('/').pop()
    return `<a class="doc" href="${esc(d.arquivo)}" target="_blank" rel="noopener">
      <span class="ic">${ic[key] || '📄'}</span>
      <span><div class="t">${esc(d.nome)}</div><div class="s">PDF · abrir${
        key === 'enredo.pdf' && ATUALIZADO_BR ? ` · atualizado em ${ATUALIZADO_BR}` : ''
      }</div></span></a>`
  }).join('')
  return `
    <section class="blk" id="docs">
      <div class="wrap">
        <h2>Os documentos <span class="sub">· compostos no browser (tex.wasm)${
          ATUALIZADO_BR ? ` · última atualização ${ATUALIZADO_BR}` : ''
        }</span></h2>
        <div class="rule"></div>
        <div class="docs">${items}</div>
      </div>
    </section>`
}

function hero(m) {
  // cada kernel declara a OPERAÇÃO da tríade que é — e o que não é nenhuma fica dito
  const chips = m.kernels.map((k) => {
    const nome = typeof k === 'string' ? k : k.nome
    const tri  = typeof k === 'string' ? '' : (k.triade || '')
    const fora = /NÃO é operação/.test(tri)
    return `<span class="chip${fora ? ' fora' : ''}" title="${esc(tri)}">${esc(nome)}</span>`
  }).join('')
  const npecas = m.secoes.reduce((a, s) => a + s.pecas.length, 0) + 1
  return `
    <header class="hero"><div class="wrap herowrap">
      <div class="herotext">
        <p class="eyebrow">Uma partida infinita · nascida de teoremas, computada no metal</p>
        <h1>${esc(m.titulo)}<span class="en">${esc(m.subtitulo)}</span></h1>
        <p class="lede">Cada personagem é um <b>operador</b>; cada mundo, um <b>corpo</b>; cada item, uma <b>régua</b>.
          Nenhum foi desenhado — todos foram <b>computados</b>, e <b>certificados</b> (resíduo 0). Um lance não é um
          movimento: é uma <b>operação</b> — ⊕ (a soma), ⊗ (o produto), ∏ (o operador) — escrita na ISA da máquina.
          E um universo não se ganha: <b>fecha-se</b>, quando nada nele vaza (o casamento, Γ=0). Aqui o coração pulsa
          agora: ${npecas} peças, nenhum <i>asset</i> alheio. O corpo é o <b>relógio</b> — a fase, em ponto
          fixo (2<sup>20</sup>), com o <i>wrap</i> a ser um AND: o ciclo fecha sem resíduo. Quem desenha é roupa.</p>
        <div class="chips">${chips}</div>
        <div class="cta">
          <a class="btn prim" href="/docs/enredo.pdf" target="_blank" rel="noopener">📜 O Enredo${
            ATUALIZADO_BR ? ` <span class="quando">· ${ATUALIZADO_BR}</span>` : ''
          }</a>
          <a class="btn ghost" href="#trailer">▶ Ver o trailer</a>
          <a class="btn ghost" href="/repo.git" target="_blank" rel="noopener"
             title="git clone https://goldenkingdom.patriatechnology.com/repo.git">⑂ O repositório</a>
        </div>
      </div>
      <div class="heroart motor" data-peca="coracao_revela" aria-label="O coração do reino"></div>
    </div></header>`
}

function autor(a) {
  if (!a) return ''
  const docs = (a.docs || []).map((d) => `<a class="doc" href="${esc(d.arquivo)}" target="_blank" rel="noopener">
      <span class="ic">\u{1F464}</span>
      <span><div class="t">${esc(d.nome)}</div><div class="s">PDF \u00b7 abrir</div></span></a>`).join('')
  return `
    <section class="blk" id="autor">
      <div class="wrap">
        <h2>O autor <span class="sub">\u00b7 quem faz, e o que procura</span></h2>
        <div class="rule"></div>
        <p><b>${esc(a.nome)}</b> \u2014 ${esc(a.papel)}</p>
        <p>${esc(a.texto)}</p>
        <p>${esc(a.estado)}</p>
        <div class="docs">${docs}</div>
        <p><a href="mailto:${esc(a.email)}">${esc(a.email)}</a>
           \u00b7 <code>git clone ${esc(a.repo)}</code></p>
      </div>
    </section>`
}

function nav(m) {
  const links = [...m.secoes.map((s) => ({ id: s.id, t: s.titulo })),
    { id: 'trailer', t: 'Trailer' }, { id: 'substratos', t: 'Substratos' }, { id: 'pontes', t: 'Pontes' }, { id: 'autor', t: 'O autor' }, { id: 'docs', t: 'Docs' }]
  // o botão ☰ só aparece no telemóvel (o CSS o esconde no desktop); abre a gaveta dos links
  return `<nav class="nav">
    <a class="brand" href="#top">Reino Dourado</a>
    <button class="navtoggle" aria-label="abrir o menu das secções" aria-expanded="false" aria-controls="navlinks">☰</button>
    <div class="links" id="navlinks">${links.map((l) => `<a class="link" href="#${l.id}">${esc(l.t)}</a>`).join('')}</div>
  </nav>`
}

// ── montagem ──
const app = document.getElementById('app')
app.appendChild(el(nav(manifesto)))
const main = el('<div id="top"></div>')
main.insertAdjacentHTML('beforeend', hero(manifesto))
manifesto.secoes.forEach((s) => main.insertAdjacentHTML('beforeend', secao(s)))
main.insertAdjacentHTML('beforeend', trailer(manifesto.trailer))
main.appendChild(initSubstratos(substratosData))
main.appendChild(initPontes(pontesData))
main.insertAdjacentHTML('beforeend', docs(manifesto.docs))
main.insertAdjacentHTML('beforeend', autor(manifesto.autor))
main.insertAdjacentHTML('beforeend', `
  <footer><div class="wrap">
    O que desenha é um <b>corpo do catálogo</b> — o mórfico: a adjunção δ⊣ε, com a dilatação a compor por
    Minkowski e o raio a <b>somar</b>. Por isso é inteiro e corre <b>na máquina</b>, sem vírgula flutuante.
    O relógio é a fase em ponto fixo, com o <i>wrap</i> a ser um AND: o ciclo fecha sem resíduo. A máquina
    tem uma instrução — <b>MOVE(slot, sentido)</b>, um código e um sinal — e o estado vive no disco.
    As réguas são teoremas com dentes —
    φ²=φ+1 (a Rainha), σ_m=m+1/σ_m (o Rei), σ₁·σ₁′=−1 (o Príncipe ⋈ Dark
    Pontryagin), s²+c²=1 (a captura <b>é</b> rotação: o Pégaso voa sem cair, e o mate não mata), a garrafa de Koch
    (perímetro ∞, área finita), o infinito exato em ℤ[φ] (φ²=φ+1 fecha Σφ⁻ʲ em dois inteiros).
    <b>A arte é a matemática, visível — e o jogo é a matemática, jogável.</b>${
      ATUALIZADO_BR ? `<span class="quando">O Enredo — última atualização em ${ATUALIZADO_BR}.</span>` : ''
    }
  </div></footer>`)
app.appendChild(main)

// ── o HEADER RESPONSIVO: a gaveta dos links (só no telemóvel; o CSS esconde o botão no desktop) ──
const navEl = document.querySelector('.nav')
const navBtn = document.querySelector('.navtoggle')
if (navEl && navBtn) {
  const fecha = () => { navEl.classList.remove('aberto'); navBtn.setAttribute('aria-expanded', 'false'); navBtn.textContent = '☰' }
  navBtn.addEventListener('click', () => {
    const abriu = navEl.classList.toggle('aberto')
    navBtn.setAttribute('aria-expanded', String(abriu))
    navBtn.textContent = abriu ? '✕' : '☰'
  })
  navEl.querySelectorAll('a.link').forEach((a) => a.addEventListener('click', fecha))   // navegou: fecha
  document.addEventListener('keydown', (e) => { if (e.key === 'Escape') fecha() })
  window.addEventListener('resize', () => { if (window.innerWidth > 980) fecha() })     // voltou ao desktop
}

// ── DOCS: PDF no browser via tex.wasm (não /docs/*.pdf estático do nginx) ──
// O Aarão: «é tradutor via wasm» — local e produção o mesmo caminho. O CV fica
// estático (não entra no corpo do tradutor); o resto compõe-se ao clicar.
document.addEventListener('click', async (ev) => {
  const a = ev.target.closest && ev.target.closest('a[href*="/docs/"]')
  if (!a) return
  const id = idDeArquivo(a.getAttribute('href'))
  if (!id) return // cv.pdf e o que não está no DOCS do tradutor
  ev.preventDefault()
  // tab aberta AQUI (síncrono) — depois do await o popup blocker fecha a porta
  const janela = window.open('about:blank', '_blank')
  const antes = a.querySelector('.s')
  const rotulo = antes ? antes.textContent : ''
  if (antes) antes.textContent = 'a compor no browser…'
  a.setAttribute('aria-busy', 'true')
  try {
    const { ms, disco, estrela, torre } = await abrirDoc(id, janela)
    if (antes) {
      const origem = disco && disco.origem ? disco.origem.replace('fetch+LS', 'LS') : '?'
      const miss = disco && disco.poeN != null ? disco.poeN : '?'
      const bit = disco && disco.bit1 ? ' · 1 bit' : ''
      const dim = estrela && estrela.dim != null ? ` · dim ${estrela.dim}` : ''
      const iface = estrela && estrela.iface != null ? ` · if ${estrela.iface}` : ''
      const induc = estrela && estrela.induc != null ? ` · ind ${estrela.induc}` : ''
      const gentil = estrela && estrela.gentil ? ' · Gentil' : ''
      const regua = estrela && estrela.reguaC != null ? ` · C${estrela.reguaC}` : ''
      const td = torre && torre.torreDim != null ? ` · torre ${torre.torreDim}` : ''
      antes.textContent = `PDF · ${ms} ms · ${origem} · miss ${miss}${dim}${iface}${induc}${gentil}${regua}${td}${bit}`
    }
  } catch (e) {
    console.error('[tex.wasm]', e)
    if (janela && !janela.closed) janela.close()
    if (antes) antes.textContent = rotulo || 'PDF · abrir'
    alert('O tradutor WASM não compôs este documento:\n' + (e && e.message ? e.message : e))
  } finally {
    a.removeAttribute('aria-busy')
  }
})

// reveal on scroll
const io = new IntersectionObserver((es) => es.forEach((e) => { if (e.isIntersecting) { e.target.classList.add('in'); io.unobserve(e.target) } }), { threshold: 0.12 })
document.querySelectorAll('.card').forEach((c) => io.observe(c))

// nav ativa conforme a seção visível
const secs = [...document.querySelectorAll('section[id], header[id]')]
const links = new Map([...document.querySelectorAll('.nav a.link')].map((a) => [a.getAttribute('href').slice(1), a]))
const navio = new IntersectionObserver((es) => es.forEach((e) => {
  if (e.isIntersecting) { links.forEach((a) => a.classList.remove('active')); links.get(e.target.id)?.classList.add('active') }
}), { rootMargin: '-45% 0px -50% 0px' })
secs.forEach((s) => navio.observe(s))

// O CAMINHO LIVRE (sem bufferização): os dois .wasm carregam PRIMEIRO (o painel + a torre de Koch), depois
// os consumidores se registram, e por fim o RELÓGIO ÚNICO parte — a fase avança no topo do quadro e todos
// (o motor o campo, os cards) a leem já pronta, no mesmo quadro. Um só rAF, um só fluxo.
// O coração é DOURADO (o sol da cena). São DOIS: o coração e o seu DUAL, em ANTIFASE (o dipolo) — esse par é
// o relógio-mestre, e a sua dinâmica comanda os demais (os cards seguem a mesma fase, bit a bit).
// A DEFASAGEM DO MOTOR: 180° EXATO (π) — a antifase PURA, a conservação (A+B=1 constante). NÃO um gap que eu
// planto (φ, ou 179° discreto): a antifase pura sozinha estaria em equilíbrio, e é o REI — o dual, o gap
// CONTÍNUO (a transformação com o sinal contrário) — quem move o relógio, organicamente. Só ele faz isso.
const _ANTIFASE = Math.PI
// o DIPOLO numa cena só, o par dual σ₁·σ₁′=−1: o PRÍNCIPE (ouro luminoso, a luz/a exp de Pontryagin) e o
// DARK PONTRYAGIN (índigo profundo, a sombra/o operador dual) — quente ⋈ frio, na MESMA marcha do corpo (u_cor + u_corB)
const _assCoracao = { c200: 9 / 200, cor: [1.00, 0.80, 0.32], corB: [0.34, 0.26, 0.60], luz: [0.55, 0.60, 0.82] }

// o <img> que sobreviveu aos motores é fallback de verdade (sem WebGL2): ganha o src e o host perde a reserva
function revelaFallback () {
  document.querySelectorAll('img[data-motor]').forEach((img) => {
    img.closest('.motor')?.classList.remove('motor')
    img.src = img.dataset.src
  })
}

initMotorWasm().then(() => {
  // o HERO: os DOIS corações iniciais na MESMA cena — a mesma marcha do corpo, mesma luz/céu/sombra. O coração (fase φ) e
  // o seu dual (fase φ + defasagem áurea) lado a lado; o gap áureo entre eles move o relógio. Pulso FRACTAL.
  const heroart = document.querySelector('.heroart')
  if (heroart) initMotorCampo(heroart, _assCoracao, _ANTIFASE)
  // a seção "O coração": o card roda a mesma cena do dipolo (o mesmo relógio-mestre)
  document.querySelectorAll('#coracao .frame').forEach((fr) => initMotorCampo(fr, _assCoracao, _ANTIFASE))
  // o TRAILER é a cena SIMPLES em tempo real, no relógio-mestre —
  // os dois corações duais e o fio do Rei, percorrendo os 4 lances pela fase (o coração, a fratura, a batalha, o mate).
  const screen = document.querySelector('.trailer .screen')
  if (screen) initTrailerCampo(screen)
  // os CARDS em cena: o PÉGASO (o rotor, s²+c²=1) e A BATALHA (a captura É rotação — nada se perde).
  // A assinatura é certificada (assinatura_cards_cena.py 4/4); o motor a revela no relógio-mestre. Antes do
  // initVelocidade: eles são motor.
  initCardsCampo()
  initCardsKernel()          // os cards que são KERNEL (o kernel emitido pela broca) entram no relógio
  // O FALLBACK, e só ele: onde o motor subiu, o <img> já saiu do DOM (o canvas o substituiu) e não há ficheiro a baixar. O <img> que SOBROU é de quem não tem WebGL2 — a esse, e só a esse, devolve-se o src.
  revelaFallback()
  // o circuito síncrono dos cards (context2d) — o mesmo relógio, comandado pelo dipolo (FP=1). Corre DEPOIS do
  // no circuito como os outros.
  initVelocidade()
  // o RELÓGIO ÚNICO: avança a fase (o painel, bit a bit) e chama os consumidores — o caminho livre
  iniciaRelogio(avancaFase)
}).catch((e) => {                       // o WASM não subiu: ninguém assumiu os hosts — as molduras ficam vazias e nomeadas
  console.warn('[reino] o motor não subiu:', e)
  revelaFallback()
})
