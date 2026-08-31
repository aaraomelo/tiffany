// manifesto_loader.js — fonte única do manifesto ISA (sem bundler; fetch em runtime).
// Não é app/src/manifesto.json (site Reino Dourado). Motor = banco/sql.c.
// Offline: LS (gk:banco:manifesto) se o fetch falhar.

import { discoBrowser, gravaManifestoLS, leManifestoLS } from './banco_disco.js'

let _manifesto = null

/** Carrega conecthus/backends/manifesto.json uma vez. Inclui linguagens, orbitas, hopfield, corpos. */
export async function carregaManifesto (url = '/conecthus/backends/manifesto.json', storage) {
  if (_manifesto) return _manifesto
  const disco = storage !== undefined ? storage : discoBrowser()
  try {
    const r = await fetch(url)
    if (!r.ok) throw new Error('manifesto: ' + r.status)
    _manifesto = await r.json()
    gravaManifestoLS(disco, _manifesto)
    return _manifesto
  } catch (e) {
    const cached = leManifestoLS(disco)
    if (cached) {
      _manifesto = cached
      return _manifesto
    }
    throw e
  }
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

/** Schema canónico de U (JSON). Não é Alonzo nem uma Parte. */
export function schemaDoUniversal () {
  const m = corposDoManifesto() && corposDoManifesto().motor
  return (m && m.schema) || 'conecthus/schema/u.schema.json'
}

/** Ponte Manifesto ↔ U. M é projecção; U é o nodo. */
export function ponteManifestoU () {
  const m = corposDoManifesto() && corposDoManifesto().motor
  return (m && m.ponte_u) || 'app/src/banco_manifesto_u.js'
}

/** Ponte página web ↔ U (tríade html/css/js). Mesmo schema, não segundo $id. */
export function pontePaginaU () {
  const m = corposDoManifesto() && corposDoManifesto().motor
  return (m && m.ponte_pagina) || 'app/src/banco_pagina_u.js'
}
