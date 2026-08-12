// ── A PORTA DA ESTRELA: um disco, dois motores (painel + tex) ──────────────
//
// Contrato único (arquitetura.tex + motor_wasm + tex_tradutor):
//   1. o host escreve na vista da memória (slots)
//   2. chama o módulo (prog / compila_ficheiro)
//   3. lê os slots de volta — sem buffer-cópia entre lados
//
// MOVE(slot, sentido) no tex.wasm: −1 emite (garante endereço), +1 absorve
// (só o nascido), 0 atravessa. O painel chessc não exporta MOVE: os índices
// do BigInt64Array SÃO os slots; escreve → prog() → lê = o mesmo par ±1.
//
// Lei 7 = circuito tex(4)–hexal–pdf(4) = octonião dual ℍ×ℍ*.
// Lei 8 = selo Caelum / AssinaturaOito (N=2⁸) — não confundir com a 7.
// Banco = LS/Map (cliente). Estrela = interface que reverte (1 bit).

/** Escreve → chama → devolve a vista. Painel: view=BigInt64Array, prog=exports.prog. */
export function chamaNoDisco ({ view, prog, escreve }) {
  if (typeof escreve === 'function') escreve(view)
  const r = typeof prog === 'function' ? prog() : undefined
  return { view, r }
}

/** +1 absorve — endereço do slot se já nasceu (PDF após marca_saida). */
export function absorve (E, slot) {
  const m = E && E.MOVE
  if (typeof m !== 'function') return 0
  const x = m(slot, 1)
  return typeof x === 'bigint' ? Number(x) : Number(x)
}

/** −1 emite — garante endereço (banco prende; rascunho nasce). */
export function emite (E, slot) {
  const m = E && E.MOVE
  if (typeof m !== 'function') return 0
  const x = m(slot, -1)
  return typeof x === 'bigint' ? Number(x) : Number(x)
}

/** Trial 0 — atravessa sem nascer. */
export function atravessa (E, slot) {
  const m = E && E.MOVE
  if (typeof m !== 'function') return 0
  const x = m(slot, 0)
  return typeof x === 'bigint' ? Number(x) : Number(x)
}
