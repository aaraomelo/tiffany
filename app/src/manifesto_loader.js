// manifesto_loader.js — fonte única do manifesto (sem bundler; fetch em runtime).

let _manifesto = null

/** Carrega conecthus/backends/manifesto.json uma vez. */
export async function carregaManifesto (url = '/conecthus/backends/manifesto.json') {
  if (_manifesto) return _manifesto
  const r = await fetch(url)
  if (!r.ok) throw new Error('manifesto: ' + r.status)
  _manifesto = await r.json()
  return _manifesto
}

export function manifestoAtual () {
  if (!_manifesto) throw new Error('manifesto não carregado — chame carregaManifesto() primeiro')
  return _manifesto
}

export function reiniciaManifesto () {
  _manifesto = null
}
