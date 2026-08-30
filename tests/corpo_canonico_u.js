/* tests/corpo_canonico_u.js — Def. corpo canónico de U: mapa, não fusão.
 *
 * Lê univ:def:corpo-canonico. Não reescreve línguas nem Alonzo.
 *   node tests/corpo_canonico_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const TEX = join(RAIZ, 'corpo_universal.tex')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

ok('§K0 tex no disco', existsSync(TEX))
const tex = readFileSync(TEX, 'utf8')

ok('§K0 linguas e Alonzo mantidos (nao reescritos)',
  /\\label\{univ:def:linguas\}/.test(tex) &&
  /\\label\{univ:def:alonzo-real\}/.test(tex) &&
  /\\mathcal\{U\}_\{\\mathrm\{alg\}\}/.test(tex) &&
  /Fractal n[aã]o [eé] quarta l[ií]ngua/.test(tex) &&
  /mathsf\{Real\}_\{\\mathrm\{fractal\}\}/.test(tex))

ok('§K1 def corpo-canonico boxed com os 3 criterios',
  /\\label\{univ:def:corpo-canonico\}/.test(tex) &&
  /fecho triplo/.test(tex) &&
  /rho_C/.test(tex) &&
  /operatorname\{realize\}/.test(tex) &&
  /cong_\{\\mathcal\{U\}\}/.test(tex) &&
  /face/.test(tex) && /fibra/.test(tex) &&
  /Lei~7/.test(tex))

ok('§K1 nec+suf != identidade de objectos',
  /nec\+suf/.test(tex) &&
  /identidade de objectos/.test(tex) &&
  /C=\\mathcal\{U\}/.test(tex) &&
  /tudo [eé] a mesma coisa/.test(tex))

ok('§K2 criterio (2) cita a adjuncao do paper morfico',
  /delta\\dashv\\varepsilon/.test(tex) &&
  /teo:adjuncao/.test(tex) &&
  /thm:morf-par/.test(tex) &&
  /def:morfico/.test(tex) &&
  /delta\\varepsilon/.test(tex) &&
  /B\\\)-abertos/.test(tex))

ok('§K2 rho cita erosao; realize cita dilatacao',
  /cita a eros/.test(tex) &&
  /cita a dilat/.test(tex) &&
  /operatorname\{realize\}\\circ\\rho_C/.test(tex))

ok('§K3 Alonzo satisfaz os 3; Fractal nao reabre',
  /Alonzo j[aá] satisfaz os tr[eê]s/.test(tex) &&
  /cat:audit:alonzo/.test(tex) &&
  /N[aã]o se reabre Fractal/.test(tex) &&
  /univ:def:alonzo-real/.test(tex))

ok('§K3 candidatos testam os 3; Docker nao sobe',
  /M_\{\\mathrm\{Docker\}\}/.test(tex) &&
  /n[aã]o localizada/.test(tex) &&
  /uma l[ií]ngua n[aã]o basta/.test(tex) &&
  /univ:def:lei-local-canonica/.test(tex))

ok('§K4 I0: adjuncao != Duo; gap boxed; coincidencia N/A',
  /\\label\{univ:obs:morf-i0\}/.test(tex) &&
  /univ:def:star/.test(tex) &&
  /fis:def:duomorf/.test(tex) &&
  /fis:thm:dual-involutiva/.test(tex) &&
  /mathcal\{D\}\\circ\\mathcal\{D\}=\\mathrm\{id\}/.test(tex) &&
  /delta\\varepsilon\\subseteq\\mathrm\{id\}\\subseteq\\varepsilon\\delta/.test(tex) &&
  /identifica[cç][aã]o [eé]/.test(tex) &&
  /N\/A/.test(tex) &&
  /n[aã]o localizada/.test(tex) &&
  /falta/.test(tex) &&
  /teo:guarda/.test(tex) &&
  /\\chi_k/.test(tex) &&
  /Dedekind/.test(tex))

ok('§K4 def nao cita Duo como o mapa nec+suf',
  /n[aã]o\} cita/.test(tex) &&
  /mathbf\{Duo\}/.test(tex) &&
  /univ:obs:morf-i0/.test(tex))

ok('§K4 morfico nao sobe a Parte; sem corpo 15',
  /teo:socorpon1/.test(tex) &&
  /thm:morfologico/.test(tex) &&
  /n[aã]o\} sobe a Parte/.test(tex) &&
  /corpo~15/.test(tex) &&
  /N[aã]o h[aá] quarta l[ií]ngua/.test(tex) &&
  /N[aã]o h[aá] Lei~8/.test(tex) &&
  /N[aã]o h[aá] Ficha~11/.test(tex))

ok('§K4 linguas-recusa intacta (EM sem corpo)',
  /\\label\{univ:obs:linguas-recusa\}/.test(tex) &&
  /N[aã]o se inventa l[ií]ngua EM/.test(tex) &&
  /fis:def:em-forma/.test(tex))

console.log('')
console.log('  corpo canonico = mapa (adjuncao δ⊣ε); nec+suf != identidade')
console.log('  recusas: fusao C=U, Duo=erosao, Lei 8, 4a lingua, corpo 15')
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
