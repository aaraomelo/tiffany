/* banco_shell_parse.mjs — literais e NODE MOVE / BASH MOVE no SQL Tiffany. */
export const S_CANAL_BASE = 9895936
export const SLOTS = {
  bash: { in: 9100, out: 9101, label: 'BASH' },
  powershell: { in: 9110, out: 9111, label: 'POWERSHELL' },
  node: { in: 9120, out: 9121, label: 'NODE' },
}

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
