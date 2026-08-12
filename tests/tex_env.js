/* env do tex.wasm: __fich_miss (fopen → 1 ficheiro). Stub = 0; o browser põe o Map. */
'use strict'

function precisaFichMiss (mod) {
  return WebAssembly.Module.imports(mod).some(
    (i) => i.module === 'env' && i.name === '__fich_miss')
}

/** Instancia tex.wasm; `fichMiss(ptrNome)→0|1` opcional (síncrono). */
function instanciaTex (bytes, fichMiss) {
  const mod = bytes instanceof WebAssembly.Module ? bytes : new WebAssembly.Module(bytes)
  if (!precisaFichMiss(mod)) return new WebAssembly.Instance(mod)
  const fn = typeof fichMiss === 'function' ? fichMiss : function () { return 0 }
  return new WebAssembly.Instance(mod, { env: { __fich_miss: fn } })
}

/** ../gkcapa / gkcapa → gkcapa.tex no Map (a mesma régua do acha_ficheiro). */
function hitCorpo (cache, nome) {
  if (cache.has(nome)) return { nome, u8: cache.get(nome) }
  const stem = (s) => {
    let b = String(s).replace(/^(\.\.\/)+/, '')
    const i = b.lastIndexOf('/')
    if (i >= 0) b = b.slice(i + 1)
    if (b.endsWith('.tex')) b = b.slice(0, -4)
    return b
  }
  const want = stem(nome)
  for (const c of [String(nome).replace(/^(\.\.\/)+/, ''), want, want + '.tex']) {
    if (cache.has(c)) return { nome: c, u8: cache.get(c) }
  }
  for (const [k, u8] of cache) if (stem(k) === want) return { nome: k, u8 }
  return null
}

module.exports = { instanciaTex, precisaFichMiss, hitCorpo }
