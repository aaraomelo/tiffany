/* lib/peano.js — a realização Peano: o primeiro corpo concreto da
 * infraestrutura Universal (𝒫 = 𝒰[σ_Peano]).
 *
 * A assinatura σ_Peano fornece o que é DA INSTÂNCIA:
 *   - o anel ℤ_65537 (o primo de Fermat da casa);
 *   - a leitura do corpo: bytes UTF-8;
 *   - o endereço: o id do registo JSON (o corrompido perde o lugar e
 *     ganha um endereço sentinela único por posição);
 * O Universal (lib/universal.js) não conhece nada disto — recebe σ.
 *
 * Semântica byte a byte a dos medidores atestados
 * (equivalencia_universal.js 7:0; cristal_volta.js 16:0).
 */
'use strict'

const sigmaPeano = {
  p: 65537,
  bytes: s => Buffer.from(String(s), 'utf8'),
  endereco: (l, i) => {
    try { return JSON.parse(l).id } catch { return '￿ corrompido ' + i }
  },
}

module.exports = { sigmaPeano }
