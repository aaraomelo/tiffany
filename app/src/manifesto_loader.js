// manifesto_loader.js — fonte única do manifesto ISA (sem bundler; fetch em runtime).
// Não é app/src/manifesto.json (site Reino Dourado). Motor = banco/sql.c.

let _manifesto = null

/** Carrega conecthus/backends/manifesto.json uma vez. Inclui linguagens, orbitas, hopfield, corpos. */
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

/** Censo das Partes / bestiário. Vazio se o manifesto ainda não carregou. */
export function corposDoManifesto () {
  return (_manifesto && _manifesto.corpos) || null
}

/** O motor ISA — banco/sql.c. Não é motor_campo nem tests/motor.c. */
export function motorDoManifesto () {
  const c = corposDoManifesto()
  return (c && c.motor) || null
}

export function corpoPorParte (parte) {
  const lista = (corposDoManifesto() && corposDoManifesto().lista) || []
  return lista.find((c) => c.parte === parte) || null
}
