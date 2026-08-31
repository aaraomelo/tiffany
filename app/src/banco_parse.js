// banco_parse.js — parser MOVE partilhado (realização wasm e remota).

export function parseLiteral (text) {
  const m = text.match(/^'((?:''|[^'])*)'/)
  if (!m) return null
  return m[1].replace(/''/g, "'")
}

export function parseMove (text, label) {
  const re = new RegExp(`^${label}\\s+MOVE(?:\\s+([+-]1))?\\s*(.*)$`, 'i')
  const m = text.match(re)
  if (!m) return null
  const sentido = m[1] === '+1' ? +1 : m[1] === '-1' ? -1 : 0
  const rest = (m[2] || '').trim()
  const script = rest ? parseLiteral(rest) : null
  if (rest && script === null && sentido !== +1) return { err: 'literal' }
  return { sentido, script }
}

export const SHELL_LABELS = {
  bash: 'BASH',
  powershell: 'POWERSHELL',
  node: 'NODE',
}

export function parseShellMove (query) {
  const q = String(query || '').trim()
  for (const [nome, label] of Object.entries(SHELL_LABELS)) {
    const mv = parseMove(q, label)
    if (!mv) continue
    if (mv.err) throw new Error(label + ' MOVE literal inválido')
    return { nome, label, ...mv }
  }
  return null
}
