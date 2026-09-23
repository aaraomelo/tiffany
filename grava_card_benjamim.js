#!/usr/bin/env node
// grava_card_benjamim.js — grava GIF do card Benjamim "O Quiral" do Reino Dourado
// Abordagem: screenshots do canvas via toDataURL(), depois ffmpeg → GIF.
// Não modifica o app — script externo.
//
// Uso: node grava_card_benjamim.js [duracao_seg] [fps] [dpr]
//   padrão: 4s 15fps 1.0

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

async function main () {
  const DUR = parseInt(process.argv[2]) || 4
  const FPS = parseInt(process.argv[3]) || 15
  const DPR = parseFloat(process.argv[4]) || 1.0
  const OUT = path.join(process.cwd(), 'gifs', `benjamim_quiral_${DUR}s_${FPS}fps.gif`)
  const sleep = (ms) => new Promise(r => setTimeout(r, ms))

  const ppPath = pathToFileURL(path.join(process.cwd(), 'app', 'node_modules', 'puppeteer', 'lib', 'puppeteer', 'puppeteer.js')).href
  const puppeteer = await import(ppPath)
  const chrome = findChrome()
  if (!chrome) { console.error('chrome não encontrado'); process.exit(1) }
  console.log(`[grava] card Benjamim "O Quiral" | ${DUR}s @ ${FPS}fps DPR=${DPR}`)
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

  console.log('[3/4] esperando card Benjamim...')
  await page.waitForSelector('.frame[data-peca="benjamim"]', { timeout: 30000 })
  await page.waitForFunction(
    () => document.querySelector('.frame[data-peca="benjamim"] canvas') !== null,
    { timeout: 30000 }
  )
  await sleep(500)

  // clica no card pra ativar a animação
  await page.evaluate(() => {
    const f = document.querySelector('.frame[data-peca="benjamim"]')
    f?.scrollIntoView({ block: 'center' })
    f?.click()
  })
  await sleep(1500) // espera a animação do canvas estabilizar

  if (!existsSync(TMP)) mkdirSync(TMP, { recursive: true })

  console.log(`[4/4] capturando ${DUR * FPS} frames...`)
  const intervalMs = 1000 / FPS
  for (let i = 0; i < DUR * FPS; i++) {
    const dataUrl = await page.evaluate(() => {
      const c = document.querySelector('.frame[data-peca="benjamim"] canvas')
      return c ? c.toDataURL('image/png') : null
    })
    if (!dataUrl) { console.error('canvas sumiu', i); break }
    writeFileSync(path.join(TMP, `frame_${String(i).padStart(5, '0')}.png`), Buffer.from(dataUrl.split(',')[1], 'base64'))
    if (i < DUR * FPS - 1) await sleep(intervalMs)
  }
  await browser.close()

  const frames = readdirSync(TMP).filter(f => f.endsWith('.png')).sort()
  console.log(`[grava] ${frames.length} frames`)

  if (frames.length < 2) { console.error('poucos frames'); process.exit(1) }

  const outDir = dirname(OUT)
  if (!existsSync(outDir)) mkdirSync(outDir, { recursive: true })
  const palettePath = path.join(TMP, 'palette.png')

  console.log('[grava] convertendo → GIF...')
  execSync(`ffmpeg -y -i "${path.join(TMP, 'frame_%05d.png')}" -vf "fps=${FPS},scale=640:-1:flags=lanczos,palettegen=max_colors=128" "${palettePath}"`, { stdio: 'pipe', timeout: 60000 })
  execSync(`ffmpeg -y -i "${path.join(TMP, 'frame_%05d.png')}" -i "${palettePath}" -filter_complex "fps=${FPS},scale=640:-1:flags=lanczos[x];[x][1:v]paletteuse=dither=bayer" -loop 0 "${OUT}"`, { stdio: 'pipe', timeout: 120000 })

  const sizeKB = existsSync(OUT) ? (readFileSync(OUT).length / 1024).toFixed(0) : '?'
  console.log(`[grava] ✅ ${OUT} (${sizeKB} KB, ${frames.length} frames)`)

  for (const f of frames) { try { unlinkSync(path.join(TMP, f)) } catch {} }
  try { unlinkSync(palettePath) } catch {}
  try { rmdirSync(TMP) } catch {}
}

main().catch(e => { console.error('[grava] ERRO:', e.message); process.exit(1) })