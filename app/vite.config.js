import { defineConfig } from 'vite'
import { fileURLToPath, URL } from 'node:url'
import { execSync } from 'node:child_process'
import { statSync } from 'node:fs'

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

// publicDir aponta para ../figuras: o dev server serve /reino/*.png e /docs/*.pdf
// direto dos assets rasterizados na GPU (sem cópia, sem base64). base relativa p/ o build.
export default defineConfig({
  base: './',
  publicDir: fileURLToPath(new URL('../figuras', import.meta.url)),
  server: { open: true },
  define: { __ENREDO_ATUALIZADO__: JSON.stringify(dataEnredo()) },
})
