/* tests/manifesto_corpos.js — censo dos corpos canónicos no manifesto.
 *
 * DEPENDE-DE: conecthus/backends/manifesto.json
 *
 * I0: língua ≠ palco ≠ corpo canónico. Não promove linguagens a Partes.
 *     Órbitas ficam sql/latex/node. Alonzo não é língua. EM não inventa corpo.
 *
 *   node tests/manifesto_corpos.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const man = JSON.parse(readFileSync(
  join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const corpos = man.corpos
ok('§C0 manifesto.corpos existe', !!corpos && Array.isArray(corpos.lista))

const lista = corpos?.lista || []
const palcos = lista.filter((c) => c.camada === 'Fisica')
ok('§C0 palcos = 14 (camada Fisica)',
  corpos?.palcos === 14 && palcos.length === 14)

const nomesPalco = palcos.map((c) => c.parte)
ok('§C0 as catorze Partes do cat:indice', [
  'Algebra', 'Topologia', 'Analise', 'Fractal', 'Geometria', 'Mecanica',
  'Gravitacao', 'Relatividade', 'Eletromagnetismo', 'Optica', 'Particulas',
  'Termodinamica', 'Cosmologia', 'Redes',
].every((p) => nomesPalco.includes(p)))

/* §C1 — cada medidor apontado existe no disco */
{
  let todos = true
  const vistos = []
  for (const c of lista) {
    for (const p of c.medidores || []) {
      if (!p || p === 'nao localizada') continue
      const abs = join(RAIZ, p)
      if (!existsSync(abs)) {
        todos = false
        console.log('#UNIT falha §C1 falta ' + p)
        falhas++
        feitas++
      } else {
        vistos.push(p)
      }
    }
  }
  ok('§C1 todos os medidores de corpos existem (' + vistos.length + ' paths)', todos)
}

/* §C2 — I0: línguas não são Partes */
{
  const linguas = (man.linguagens || []).map((l) => l.nome)
  const partes = palcos.map((c) => String(c.parte).toLowerCase())
  const choque = linguas.filter((n) => partes.includes(String(n).toLowerCase()))
  ok('§C2 nenhuma lingua tem o nome de uma Parte', choque.length === 0)
  ok('§C2 Alonzo nao e lingua', !linguas.includes('alonzo') && !linguas.includes('Alonzo'))
  ok('§C2 Fractal nao e lingua', !linguas.includes('fractal') && !linguas.includes('Fractal'))
}

/* §C3 — órbitas = sql/latex/node; nenhuma quarta língua-órbita */
{
  const nomes = (man.orbitas || []).map((o) => o.nome).sort()
  ok('§C3 orbitas = sql, latex, node',
    nomes.length === 3 && nomes[0] === 'latex' && nomes[1] === 'node' && nomes[2] === 'sql')
  ok('§C3 nenhuma orbita Alonzo/Fractal',
    !nomes.includes('alonzo') && !nomes.includes('fractal') && !nomes.includes('Alonzo'))
  ok('§C3 nenhum corpo de Parte declara-se orbita',
    palcos.every((c) => c.orbita == null))
}

/* §C4 — recortes I0 do censo */
{
  const fractal = palcos.find((c) => c.parte === 'Fractal')
  ok('§C4 Fractal canonico = Alonzo', fractal && fractal.canonico === 'Alonzo')
  const em = palcos.find((c) => c.parte === 'Eletromagnetismo')
  ok('§C4 EM nao ha corpo', em && em.canonico == null)
  const redes = palcos.find((c) => c.parte === 'Redes')
  ok('§C4 Redes canonico = Corpo Neural', redes && redes.canonico === 'Corpo Neural')
  ok('§C4 Relatividade medidores vazios (nao localizada)',
    (palcos.find((c) => c.parte === 'Relatividade')?.medidores || []).length === 0)
  ok('§C4 Optica medidores vazios (nao localizada)',
    (palcos.find((c) => c.parte === 'Optica')?.medidores || []).length === 0)
}

/* §C5 — o motor é sql.c; homónimos não o substituem */
{
  const mot = corpos?.motor || {}
  ok('§C5 motor.isa = banco/sql.c', mot.isa === 'banco/sql.c')
  ok('§C5 motor.interface = sql = interface_padrao',
    mot.interface === 'sql' && man.interface_padrao === 'sql')
  ok('§C5 tests/motor.c e homonimo, nao o ISA',
    (mot.homonimos || []).includes('tests/motor.c') && mot.isa !== 'tests/motor.c')
  ok('§C5 motor_campo e roupa, nao o ISA',
    (mot.homonimos || []).includes('app/src/motor_campo.js'))
  const linguas = (man.linguagens || []).map((l) => l.nome)
  ok('§C5 shells padrao node; 19 linguas ingeridas',
    mot.shell_padrao === 'node' &&
    Array.isArray(mot.ingerido) &&
    mot.ingerido.length === 19 &&
    linguas.length === 19 &&
    mot.ingerido.every((n) => linguas.includes(n)))
  ok('§C5 asm nao esta em ingerido (degrau de isa)',
    !mot.ingerido.includes('asm') && typeof mot.asm === 'string' && /isa/.test(mot.asm))
}

/* §C9 — node: linguagem + órbita canal, não fio HTTP */
{
  const node = (man.linguagens || []).find((l) => l.nome === 'node')
  ok('§C9 node tem absorcao.orbita = canal', node && node.absorcao && node.absorcao.orbita === 'canal')
  ok('§C9 node pleno = banco/node.c', node && node.pleno === 'banco/node.c')
  ok('§C9 node e orbita, nao fio',
    (man.orbitas || []).some((o) => o.nome === 'node') &&
    !(man.fios || []).some((f) => f.nome === 'node'))
  ok('§C9 celula r=0 / metal r=1 (ingerido no pleno, nao homogeneizado com sql)',
    node && node.r === 0 && node.cadeia && node.cadeia.metal && node.cadeia.metal.r === 1)
}

/* §C10 — bash e powershell: canal ingerido, nao orbita Hopfield */
{
  function shellIngerido (nome, pleno) {
    const L = (man.linguagens || []).find((l) => l.nome === nome)
    return L && L.pleno === pleno &&
      L.absorcao && L.absorcao.orbita === 'canal' &&
      L.r === 0 && L.cadeia && L.cadeia.metal && L.cadeia.metal.r === 1 &&
      !(man.orbitas || []).some((o) => o.nome === nome) &&
      !(man.fios || []).some((f) => f.nome === nome)
  }
  ok('§C10 bash ingerido (canal, nao Hopfield)',
    shellIngerido('bash', 'banco/bash.c'))
  ok('§C10 powershell ingerido (canal, nao Hopfield)',
    shellIngerido('powershell', 'banco/powershell.c'))
}

/* §C6 — Hopfield: uma matriz, das órbitas, candidato Neural */
{
  const hp = man.hopfield || {}
  ok('§C6 hopfield.canonico = Corpo Neural', hp.canonico === 'Corpo Neural')
  ok('§C6 hopfield.estatuto = candidato', hp.estatuto === 'candidato')
  ok('§C6 hopfield.orbitas = sql/latex/node',
    Array.isArray(hp.orbitas) && hp.orbitas.join(',') === 'sql,latex,node')
  ok('§C6 hopfield.medidor existe',
    hp.medidor && existsSync(join(RAIZ, hp.medidor)))
  ok('§C6 fita G=1 nao e a matriz',
    /G=1/.test(hp.fita || '') && /nao da fita/.test(hp.fita || ''))
}

/* §C7 — matrizes: N/A onde I2 o for; cruz em Alonzo; gato na Algebra */
{
  const alg = palcos.find((c) => c.parte === 'Algebra')
  const frac = palcos.find((c) => c.parte === 'Fractal')
  const geo = palcos.find((c) => c.parte === 'Geometria')
  const mec = palcos.find((c) => c.parte === 'Mecanica')
  ok('§C7 Algebra matriz = gato |det|=1', /gato/.test(alg?.matriz || '') && /Hopfield/.test(alg?.matriz || ''))
  ok('§C7 Alonzo matriz = cruz, nao W', /cruz/.test(frac?.matriz || '') && /Nao W/.test(frac?.matriz || ''))
  ok('§C7 Geometria matriz N/A (I2)', /^N\/A/.test(geo?.matriz || ''))
  ok('§C7 Mecanica matriz N/A como magmas', /N\/A como magmas/.test(mec?.matriz || ''))
}

/* §C11 — censo ISA: 19 línguas; fonte no disco; js ≠ node; asm ∉ linguagens[] */
{
  const linguas = (man.linguagens || []).map((l) => l.nome)
  const isa = corpos?.motor?.linguagens_isa || {}
  ok('§C11 linguagens[] tem 19 chaves', linguas.length === 19)
  ok('§C11 asm nao e lingua', !linguas.includes('asm'))
  ok('§C11 js ≠ node (homonimos, dois contratos)',
    linguas.includes('js') && linguas.includes('node') &&
    (man.linguagens.find((l) => l.nome === 'js')?.faz) !==
    (man.linguagens.find((l) => l.nome === 'node')?.faz))
  ok('§C11 linguagens_isa cobre as 19 + asm como cadeia',
    linguas.every((n) => isa[n] && isa[n].absorcao === 'sim') &&
    isa.asm && isa.asm.lingua === false && isa.asm.cadeia_de === 'isa')

  let fontes = true
  for (const L of man.linguagens || []) {
    if (!L.fonte || !existsSync(join(RAIZ, L.fonte))) {
      fontes = false
      console.log('#UNIT falha §C11 fonte ' + (L.fonte || L.nome))
      falhas++
      feitas++
    }
    if (!L.wasm || !/\.wasm$/.test(L.wasm)) {
      fontes = false
      console.log('#UNIT falha §C11 wasm ' + L.nome)
      falhas++
      feitas++
    }
    const mv = L.absorcao?.move
    if (!mv || !(L.exports || []).includes(mv)) {
      fontes = false
      console.log('#UNIT falha §C11 absorcao.move ' + L.nome)
      falhas++
      feitas++
    }
    const asm = L.cadeia?.asm
    if (asm && asm !== 'nao localizada' && asm.includes('/') && !existsSync(join(RAIZ, asm))) {
      fontes = false
      console.log('#UNIT falha §C11 cadeia.asm ' + asm)
      falhas++
      feitas++
    }
    const partes = palcos.map((c) => String(c.parte).toLowerCase())
    if (partes.includes(String(L.nome).toLowerCase())) {
      fontes = false
      console.log('#UNIT falha §C11 lingua=Parte ' + L.nome)
      falhas++
      feitas++
    }
  }
  ok('§C11 fonte/wasm/move/cadeia no disco; lingua ≠ Parte', fontes)
}

/* §C8 — N-R e operacional nao invadem as Partes */
{
  ok('§C8 cadeia N-R na lista, camada N-R',
    lista.some((c) => c.canonico === 'cadeia N-R' && c.camada === 'N-R' && c.parte == null))
  ok('§C8 ISA SQL operacional, nao capitulo',
    lista.some((c) => c.canonico === 'ISA SQL' && c.camada === 'operacional'))
}

/* §C12 — schema U autossimilar; JSON canónico; parser no motor */
{
  const mot = corpos?.motor || {}
  ok('§C12 schema U no disco',
    mot.schema === 'conecthus/schema/u.schema.json' &&
    existsSync(join(RAIZ, mot.schema)))
  ok('§C12 instancia U no disco',
    mot.schema_instancia === 'conecthus/schema/u.json' &&
    existsSync(join(RAIZ, mot.schema_instancia)))
  ok('§C12 parser de ficheiros no motor',
    mot.parser === 'banco/parse_ficheiro.h' &&
    existsSync(join(RAIZ, mot.parser)))
  {
    const inst = JSON.parse(readFileSync(join(RAIZ, mot.schema_instancia), 'utf8'))
    const sch = JSON.parse(readFileSync(join(RAIZ, mot.schema), 'utf8'))
    ok('§C12 instancia star=D; autossimilar $ref',
      inst.kind === 'U' && inst.star === 'D' &&
      sch.properties?.faces?.properties?.menos?.$ref === '#')
    ok('§C12 Alonzo nao e U', inst.id !== 'Alonzo')
  }
}

/* §C13 — ponte Manifesto ↔ U: M é projecção, não segundo canónico */
{
  const mot = corpos?.motor || {}
  ok('§C13 ponte_u no disco',
    mot.ponte_u === 'app/src/banco_manifesto_u.js' &&
    existsSync(join(RAIZ, mot.ponte_u)))
  ok('§C13 medidor da ponte',
    existsSync(join(RAIZ, 'tests', 'manifesto_u.js')))
  {
    const sch = JSON.parse(readFileSync(join(RAIZ, mot.schema || 'conecthus/schema/u.schema.json'), 'utf8'))
    ok('§C13 kinds orbita/celula no schema',
      sch.properties?.kind?.enum?.includes('orbita') &&
      sch.properties?.kind?.enum?.includes('celula'))
  }
}

/* §C14 — schema da página web (mesmo U; cliente/servidor no canal) */
{
  const mot = corpos?.motor || {}
  ok('§C14 ponte_pagina no disco',
    mot.ponte_pagina === 'app/src/banco_pagina_u.js' &&
    existsSync(join(RAIZ, mot.ponte_pagina)))
  ok('§C14 instancia pagina no disco',
    mot.schema_pagina === 'conecthus/schema/pagina.json' &&
    existsSync(join(RAIZ, mot.schema_pagina)))
  ok('§C14 kind pagina no schema; html/css/js formatos',
    JSON.parse(readFileSync(join(RAIZ, mot.schema || 'conecthus/schema/u.schema.json'), 'utf8'))
      .properties?.kind?.enum?.includes('pagina'))
  ok('§C14 ponte_vite no disco (esqueleto; nao kind novo)',
    mot.ponte_vite === 'app/src/banco_vite_u.js' &&
    existsSync(join(RAIZ, mot.ponte_vite)) &&
    mot.schema_vite === 'conecthus/schema/vite.json' &&
    existsSync(join(RAIZ, mot.schema_vite)) &&
    /hospedeiro/.test(mot.nucleo?.vite_esqueleto || ''))
  ok('§C14 ponte_tenant no disco (borda; Nginx != motor)',
    mot.ponte_tenant === 'app/src/banco_tenant_u.js' &&
    existsSync(join(RAIZ, mot.ponte_tenant)) &&
    mot.schema_tenant === 'conecthus/schema/tenant.json' &&
    existsSync(join(RAIZ, mot.schema_tenant)) &&
    /tenant != id != K_i/.test(mot.nucleo?.tenant_borda || ''))
  ok('§C14 ponte_nav no disco (hash original; iframe oraculo)',
    mot.ponte_nav === 'app/src/banco_nav_u.js' &&
    existsSync(join(RAIZ, mot.ponte_nav)) &&
    mot.schema_nav === 'conecthus/schema/nav.json' &&
    existsSync(join(RAIZ, mot.schema_nav)) &&
    /iframe oraculo/.test(mot.nucleo?.nav_gk || ''))
  ok('§C14 ponte_estado_gk (nao e S_ESTADO)',
    mot.ponte_estado_gk === 'app/src/banco_estado_gk_u.js' &&
    existsSync(join(RAIZ, mot.ponte_estado_gk)) &&
    mot.schema_estado_gk === 'conecthus/schema/estado_gk.json' &&
    existsSync(join(RAIZ, mot.schema_estado_gk)) &&
    /S_ESTADO != sessao/.test(mot.nucleo?.estado_gk || ''))
}

/* §C15 — sessão remota: endereço + chave pública no mesmo U */
{
  const mot = corpos?.motor || {}
  ok('§C15 ponte_sessao no disco',
    mot.ponte_sessao === 'app/src/banco_sessao_u.js' &&
    existsSync(join(RAIZ, mot.ponte_sessao)))
  ok('§C15 kind sessao; formatos sh/ps1',
    JSON.parse(readFileSync(join(RAIZ, mot.schema || 'conecthus/schema/u.schema.json'), 'utf8'))
      .properties?.kind?.enum?.includes('sessao') &&
    JSON.parse(readFileSync(join(RAIZ, mot.schema || 'conecthus/schema/u.schema.json'), 'utf8'))
      .properties?.formato?.enum?.includes('sh'))
}

/* §C17 — identidade operacional: chave > sessão > fingerprint > oauth (censo) */
{
  const mot = corpos?.motor || {}
  const id = man.mvp?.identidade
  ok('§C17 ponte_identidade no disco',
    mot.ponte_identidade === 'app/src/banco_identidade_u.js' &&
    existsSync(join(RAIZ, mot.ponte_identidade)))
  ok('§C17 censo: chave/sessao realizados; fingerprint candidato; oauth nao localizada',
    /realizado/.test(id?.slots?.chave || '') &&
    /realizado/.test(id?.slots?.sessao || '') &&
    /candidato/.test(id?.slots?.fingerprint || '') &&
    /nao localizada/.test(id?.slots?.oauth || '') &&
    /nao e N\/A/.test(id?.slots?.oauth || ''))
  ok('§C17 identidade != Exec; sem kind identidade no schema',
    (/!= Exec/.test(id?.nota || '') || /!= Exec/.test(id?.proibicao || '')) &&
    !JSON.parse(readFileSync(join(RAIZ, mot.schema || 'conecthus/schema/u.schema.json'), 'utf8'))
      .properties?.kind?.enum?.includes('identidade'))
  ok('§C17 ciclo: bind soft; recuperacao nao localizada; S_DEPOSITO opaco',
    /soft/.test(id?.ciclo?.bind || '') &&
    /nao localizada/.test(id?.ciclo?.recuperacao || '') &&
    /9220/.test(id?.deposito || '') &&
    /deposito\.bin/.test(man.mvp?.armazenamento?.deposito || ''))
}

/* §C16 — disco local (LS) e sync remoto S_ESTADO */
{
  const mot = corpos?.motor || {}
  ok('§C16 ponte_disco no disco',
    mot.ponte_disco === 'app/src/banco_disco.js' &&
    existsSync(join(RAIZ, mot.ponte_disco)))
  ok('§C16 ponte_sync no disco',
    mot.ponte_sync === 'app/src/banco_sync.js' &&
    existsSync(join(RAIZ, mot.ponte_sync)))
  ok('§C16 ponte_maquina no disco',
    mot.ponte_maquina === 'app/src/banco_maquina_u.js' &&
    existsSync(join(RAIZ, mot.ponte_maquina)))
  ok('§C16 candidatos IDB/docker/mongo no mvp',
    man.mvp?.realizacoes?.idb?.includes('nao localizada') &&
    man.mvp?.realizacoes?.docker?.includes('nao localizada') &&
    man.mvp?.realizacoes?.mongo?.includes('nao localizada'))
  ok('§C16 ponte_varredura no disco (nao promove L_S)',
    mot.ponte_varredura === 'app/src/banco_lei_local_u.js' &&
    existsSync(join(RAIZ, mot.ponte_varredura)))
  ok('§C16 ponte_lei_unica + nucleo (sem promover Docker)',
    mot.ponte_lei_unica === 'app/src/banco_lei_unica_u.js' &&
    existsSync(join(RAIZ, mot.ponte_lei_unica)) &&
    mot.ponte_transf === 'app/src/banco_transf_u.js' &&
    /realizado/.test(mot.nucleo?.retorno || '') &&
    /nao localizada/.test(mot.nucleo?.M_Docker || ''))
  ok('§C18 ponte_cristalchain no disco',
    mot.ponte_cristalchain === 'app/src/banco_cristalchain_u.js' &&
    existsSync(join(RAIZ, mot.ponte_cristalchain)) &&
    /realizado/.test(mot.nucleo?.cristalchain || '') &&
    /blockchain/.test(mot.nucleo?.cristalchain || ''))
  ok('§C18 ponte_coord no disco (realizado WSS; nao funde com a cadeia)',
    mot.ponte_coord === 'app/src/banco_coord_u.js' &&
    existsSync(join(RAIZ, mot.ponte_coord)) &&
    mot.ponte_selo === 'app/src/banco_selo_u.js' &&
    existsSync(join(RAIZ, mot.ponte_selo)) &&
    mot.ponte_fuse === 'app/src/banco_fuse_u.js' &&
    existsSync(join(RAIZ, mot.ponte_fuse)) &&
    mot.ponte_coord_canal === 'app/src/banco_coord_canal.js' &&
    existsSync(join(RAIZ, mot.ponte_coord_canal)) &&
    /realizado/.test(mot.nucleo?.coord_distribuida || '') &&
    /consenso/.test(mot.nucleo?.coord_distribuida || '') &&
    /S_ESTADO/.test(mot.nucleo?.coord_distribuida || '') &&
    /eventual/.test(mot.nucleo?.coord_distribuida || '') &&
    /capacidades/.test(mot.nucleo?.coord_distribuida || '') &&
    /selo/.test(mot.nucleo?.coord_distribuida || '') &&
    /fuse/.test(mot.nucleo?.coord_distribuida || ''))
}

console.log('')
if (!falhas) {
  console.log('  Censo: 14 palcos no manifesto; medidores no disco; linguas ≠ Partes;')
  console.log('  orbitas = sql/latex/node; motor = banco/sql.c; 19 linguas ISA; Hopfield = candidato Neural.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
