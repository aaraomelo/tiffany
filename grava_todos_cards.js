#!/usr/bin/env node
// grava_todos_cards.js — grava GIFs de TODOS os cards do Reino Dourado
// Abordagem: screenshots do canvas via toDataURL(), depois ffmpeg → GIF.
// Não modifica o app — script externo.
//
// Uso: node grava_todos_cards.js [duracao_seg] [fps] [dpr]
//   padrão: 6s 24fps 1.0

import path, { dirname } from 'node:path'
import { pathToFileURL } from 'node:url'
import os from 'node:os'
import { execSync } from 'node:child_process'
import { existsSync, mkdirSync, writeFileSync, readdirSync, unlinkSync, rmdirSync, readFileSync } from 'node:fs'

const TMP = path.join(os.tmpdir(), `gk_frames_${Date.now()}`)

function findChrome () {
  for (const c of [
    'C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe',
    'C:\\Program Files\\Google\\Chrome Beta\\Application\\chrome.exe',
    process.env.LOCALAPPDATA + '\\Google\\Chrome\\Application\\chrome.exe',
    process.env.PROGRAMFILES + '\\Google\\Chrome\\Application\\chrome.exe',
  ]) if (existsSync(c)) return c
  return null
}

// cards que não renderizam canvas (são imagens/texto, não animação)
const SEM_CANVAS = new Set(['coracao_revela', 'captura', 'cifra_ouro', 'armadura_universal', 'garrafa_koch', 'ferramenta', 'ferramenta_meia', 'broca_caelum', 'pulso', 'clock'])

async function main () {
  const DUR = parseInt(process.argv[2]) || 6       // duração aumentada para ciclo completo
  const FPS = parseInt(process.argv[3]) || 24      // FPS aumentado para smoother
  const DPR = parseFloat(process.argv[4]) || 1.0
  const OUTDIR = path.join(process.cwd(), 'gifs')
  const sleep = (ms) => new Promise(r => setTimeout(r, ms))

  const ppPath = pathToFileURL(path.join(process.cwd(), 'app', 'node_modules', 'puppeteer', 'lib', 'puppeteer', 'puppeteer.js')).href
  const puppeteer = await import(ppPath)
  const chrome = findChrome()
  if (!chrome) { console.error('chrome não encontrado'); process.exit(1) }
  console.log(`[grava] todos os cards | ${DUR}s @ ${FPS}fps DPR=${DPR}`)
  console.log(`[grava] chrome: ${chrome}`)

  const browser = await puppeteer.default.launch({
    headless: 'new',
    executablePath: chrome,
    args: ['--use-gl=egl', '--no-sandbox', '--disable-dev-shm-usage', '--disable-gpu', '--window-size=1280,800']
  })
  const page = await browser.newPage()
  const client = await page.target().createCDPSession()
  await client.send('Emulation.setDeviceMetricsOverride', { width: 1280, height: 800, deviceScaleFactor: DPR, mobile: false })

  console.log('[2/4] abrindo app...')
  await page.goto('http://localhost:5173', { waitUntil: 'load', timeout: 30000 })
  await new Promise(r => setTimeout(r, 3000)) // espera app estabilizar

  // descobre todos os cards
  console.log('[3/4] descobrindo cards...')
  const pecas = await page.evaluate(() => {
    const frames = document.querySelectorAll('.frame[data-peca]')
    return Array.from(frames).map(f => f.dataset.peca)
  })
  console.log(`[grava] ${pecas.length} cards encontrados`)

  if (!existsSync(OUTDIR)) mkdirSync(OUTDIR, { recursive: true })

  let gravados = 0
  let pulados = 0

  // grava cada card
  for (const peca of pecas) {
    // pula cards sem canvas (não animados)
    if (SEM_CANVAS.has(peca)) {
      console.log(`[grava] ⚠ ${peca}: sem canvas (estático), pulando`)
      pulados++
      continue
    }

    const out = path.join(OUTDIR, `${peca}_${DUR}s_${FPS}fps.gif`)
    console.log(`[grava] card: ${peca}...`)

    try {
      // espera o card aparecer
      await page.waitForFunction(
        (p) => document.querySelector(`.frame[data-peca="${p}"]`) !== null,
        { timeout: 15000 }, peca
      )
      await sleep(300)

      // clica no card pra ativar animação
      await page.evaluate((p) => {
        const f = document.querySelector(`.frame[data-peca="${p}"]`)
        f?.scrollIntoView({ block: 'center' })
        f?.click()
      }, peca)
      await sleep(1500) // espera a animação estabilizar (ciclo completo da transformação)

      // verifica se canvas existe
      const hasCanvas = await page.evaluate((p) => {
        const f = document.querySelector(`.frame[data-peca="${p}"]`)
        return f?.querySelector('canvas') !== null
      }, peca)

      if (!hasCanvas) {
        console.log(`[grava] ⚠ ${peca}: sem canvas, pulando`)
        pulados++
        continue
      }

      // captura frames
      if (!existsSync(TMP)) mkdirSync(TMP, { recursive: true })
      const intervalMs = 1000 / FPS
      const totalFrames = DUR * FPS

      for (let i = 0; i < totalFrames; i++) {
        const dataUrl = await page.evaluate((p) => {
          const f = document.querySelector(`.frame[data-peca="${p}"]`)
          const c = f?.querySelector('canvas')
          return c ? c.toDataURL('image/png') : null
        }, peca)
        if (!dataUrl) { console.error(`canvas sumiu ${peca}`, i); break }
        writeFileSync(path.join(TMP, `frame_${String(i).padStart(5, '0')}.png`), Buffer.from(dataUrl.split(',')[1], 'base64'))
        if (i < totalFrames - 1) await sleep(intervalMs)
      }

      const frames = readdirSync(TMP).filter(f => f.endsWith('.png')).sort()
      if (frames.length < 2) { console.error(`poucos frames ${peca}`); continue }

      // monta GIF
      const palettePath = path.join(TMP, 'palette.png')
      execSync(`ffmpeg -y -i "${path.join(TMP, 'frame_%05d.png')}" -vf "fps=${FPS},scale=640:-1:flags=lanczos,palettegen=max_colors=128" "${palettePath}"`, { stdio: 'pipe', timeout: 60000 })
      execSync(`ffmpeg -y -i "${path.join(TMP, 'frame_%05d.png')}" -i "${palettePath}" -filter_complex "fps=${FPS},scale=640:-1:flags=lanczos[x];[x][1:v]paletteuse=dither=bayer" -loop 0 "${out}"`, { stdio: 'pipe', timeout: 120000 })

      const sizeKB = existsSync(out) ? (readFileSync(out).length / 1024).toFixed(0) : '?'
      console.log(`[grava] ✅ ${out} (${sizeKB} KB, ${frames.length} frames)`)
      gravados++

      // limpa frames
      for (const f of frames) { try { unlinkSync(path.join(TMP, f)) } catch {} }
      try { unlinkSync(palettePath) } catch {}
    } catch (e) {
      console.error(`[grava] ⚠ ${peca}: ${e.message}`)
    }
  }

  await browser.close()
  try { rmdirSync(TMP) } catch {}
  console.log(`[grava] ✅ Concluído. ${gravados} GIFs gravados, ${pulos} pulados. GIFs em: ${OUTDIR}`)
}

main().catch(e => { console.error('[grava] ERRO:', e.message); process.exit(1) })