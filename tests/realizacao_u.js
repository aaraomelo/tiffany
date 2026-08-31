/* tests/realizacao_u.js — isomorfismo operacional entre realizações da máquina.
 *
 * Mesmo S0 + mesma instrução → S' igual nas realizações com medidor.
 * M_wasm (LS): GKBANCO + memoriaLS. M_remota: contrato JSON (estado.json / C).
 * Disco IDB: ponte isomorfa (shim); custo I/O no medidor (custo != correcao).
 * M_WASM+IDB / docker / mongo = nao localizada. Fase B: Exec isolado; par WASM/volume;
 * contentor so se docker correr; Identidade intocada; S_ESTADO ⊥ S_DEPOSITO.
 *
 *   node tests/realizacao_u.js
 */
import { existsSync, readFileSync, writeFileSync, mkdirSync, rmSync } from 'node:fs'
import { spawnSync } from 'node:child_process'
import { join, dirname } from 'node:path'
import { tmpdir } from 'node:os'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import {
  MAGIA, CHAVE_ESTADO, CHAVE_MANIFESTO, chaveWasm,
  STORE_ESTADO, STORE_MANIFESTO, STORE_WASM,
  estadoVazio, mergeEstado, gravaPagina, gravaManifestoLS, leManifestoLS,
  leEstado, memoriaIDB, parteChave, escolheDisco,
} from '../app/src/banco_disco.js'
import { modoDaSessao } from '../app/src/banco_sessao_u.js'
import {
  maquinaParaU, nodoRealizacao, REALIZACOES, DISCOS,
  ESTADO_VAZIO_REMOTO, SLOTS_ESTADO, SLOTS_M, SLOTS_M_MIN,
  aplicaInstrucao, isoOperacional, cicloWasm, cicloIDB, cicloRemoto, estadoVazioRemoto,
  hashCanonico, estadoCanonico, ordemVisibilidade,
  medeIO, cicloVolume, cicloExecNativo, cicloDocker, sondaDocker,
  instrucaoSqlWasm, VOLUME_ESTADO, NULO_ARENA,
} from '../app/src/banco_maquina_u.js'
import { resolveIdentidade } from '../app/src/banco_identidade_u.js'
import { S_CANAL, S_ESTADO_REQ, S_ESTADO_RSP, S_DEPOSITO_REQ, S_DEPOSITO_RSP } from '../tools/canal_slots.mjs'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const PATRIA = join(RAIZ, 'banco', 'canal_patria.c')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const man = JSON.parse(readFileSync(MAN, 'utf8'))
const schema = JSON.parse(readFileSync(SCHEMA, 'utf8'))
const patria = readFileSync(PATRIA, 'utf8')
const M = maquinaParaU()

ok('§M0 ponte_maquina no motor',
  man.corpos?.motor?.ponte_maquina === 'app/src/banco_maquina_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_maquina)))
ok('§M0 schema $id tiffany://u (sem segundo schema)', schema.$id === 'tiffany://u')
ok('§M0 kind realizacao no schema', schema.properties.kind.enum.includes('realizacao'))

ok('§M1 maquina kind=realizacao id=maquina',
  M.kind === 'realizacao' && M.id === 'maquina' && M.estatuto === 'realizado')
ok('§M1 M=(Exec,D,C) nos slots do schema',
  M.slots?.Exec === SLOTS_M.Exec && M.slots?.D === SLOTS_M.D && M.slots?.C === SLOTS_M.C &&
  SLOTS_M.Exec === 'WASM' && SLOTS_M.D === 'disco' && SLOTS_M.C === 'canal')
ok('§M1 M_min nos slots exportados e na nota',
  SLOTS_M_MIN.Exec === 'WASM' && SLOTS_M_MIN.D === 'D_*' && SLOTS_M_MIN.Memoria === 'local' &&
  /M_min=\(WASM, D_\*, Memoria local\)/.test(M.nota || '') &&
  /M=\(Exec,D,C\)/.test(M.evidencia || ''))
ok('§M1 faces MOVE ±1', M.faces.menos.sentido === -1 && M.faces.mais.sentido === 1)
ok('§M1 wasm e remota realizados',
  M.filhos.find((f) => f.id === 'wasm')?.estatuto === 'realizado' &&
  M.filhos.find((f) => f.id === 'remota')?.estatuto === 'realizado')
ok('§M1 wasm+idb = nao localizada (candidato 1)',
  M.filhos.find((f) => f.id === 'wasm+idb')?.estatuto === 'nao localizada' &&
  REALIZACOES.find((r) => r.id === 'wasm+idb')?.estatuto === 'nao localizada' &&
  REALIZACOES.find((r) => r.id === 'wasm+idb')?.nota === 'candidato 1')
ok('§M1 docker = nao localizada (candidato 2)',
  M.filhos.find((f) => f.id === 'docker')?.estatuto === 'nao localizada' &&
  REALIZACOES.find((r) => r.id === 'docker')?.nota === 'candidato 2' &&
  REALIZACOES.find((r) => r.id === 'docker')?.slots?.Exec === 'docker' &&
  REALIZACOES.find((r) => r.id === 'docker')?.slots?.D === 'volume' &&
  REALIZACOES.find((r) => r.id === 'docker')?.slots?.C === 'canal')
ok('§M1 mongo = nao localizada (candidato 3)',
  M.filhos.find((f) => f.id === 'mongo')?.estatuto === 'nao localizada' &&
  REALIZACOES.find((r) => r.id === 'mongo')?.nota === 'candidato 3')
ok('§M1 localStorage e estado.json realizados',
  M.filhos.find((f) => f.id === 'localStorage')?.estatuto === 'realizado' &&
  M.filhos.find((f) => f.id === 'estado.json')?.estatuto === 'realizado')
ok('§M1 IndexedDB disco = mapeamento realizado (nao e a maquina)',
  M.filhos.find((f) => f.id === 'IndexedDB')?.estatuto === 'realizado' &&
  M.filhos.find((f) => f.id === 'IndexedDB')?.suporte === 'disco' &&
  DISCOS.find((d) => d.id === 'IndexedDB')?.estatuto === 'realizado')
ok('§M1 volume disco = snapshot (nao e M_Docker)',
  M.filhos.find((f) => f.id === 'volume')?.estatuto === 'realizado' &&
  M.filhos.find((f) => f.id === 'volume')?.slots?.D === 'volume' &&
  DISCOS.find((d) => d.id === 'volume')?.estatuto === 'realizado' &&
  REALIZACOES.find((r) => r.id === 'docker')?.estatuto === 'nao localizada')

{
  const extra = []
  function walk (n) {
    for (const k of Object.keys(n || {})) {
      if (!(k in schema.properties)) extra.push((n.kind || '?') + '.' + k)
    }
    if (n?.faces?.menos) walk(n.faces.menos)
    if (n?.faces?.mais) walk(n.faces.mais)
    for (const f of n?.filhos || []) walk(f)
  }
  walk(M)
  ok('§M2 maquina cabe no schema U', extra.length === 0)
}

ok('§M3 manifesto candidatos nao localizada',
  /nao localizada/.test(man.mvp?.armazenamento?.docker || '') &&
  /nao localizada/.test(man.mvp?.armazenamento?.mongo || '') &&
  man.mvp?.realizacoes?.docker?.includes('nao localizada') &&
  man.mvp?.realizacoes?.mongo?.includes('nao localizada') &&
  man.mvp?.realizacoes?.idb?.includes('nao localizada') &&
  /estado\.json/.test(man.mvp?.armazenamento?.volume || '') &&
  /paridade de contentor/.test(man.mvp?.realizacoes?.docker || ''))
ok('§M3 disco canonico localStorage',
  /localStorage/.test(man.mvp?.armazenamento?.disco || '') &&
  /IndexedDB/.test(man.mvp?.armazenamento?.disco || ''))
ok('§M3 modo nao e fallback',
  /fallback offline/.test(man.protocolo?.nota || '') &&
  /fallback offline/.test(schema.properties.modo.description || ''))
ok('§M3 TIFFANY_PUB e realizacao, nao metal',
  /realizacao remota/.test(man.protocolo?.chave || '') &&
  !/TIFFANY_PUB no metal/.test(man.protocolo?.chave || ''))
ok('§M3 cadeia.metal clarificado no motor',
  /face r=1/.test(man.corpos?.motor?.cadeia_nota || ''))

ok('§M4 slots S_ESTADO 9210/9211',
  SLOTS_ESTADO.in === 9210 && SLOTS_ESTADO.out === 9211 &&
  S_ESTADO_REQ === S_CANAL + 9210 && S_ESTADO_RSP === S_CANAL + 9211)
ok('§M4 C declara o mesmo JSON vazio',
  patria.includes(ESTADO_VAZIO_REMOTO.replace(/"/g, '\\"' )) ||
  patria.includes(ESTADO_VAZIO_REMOTO))
ok('§M4 estado vazio remoto ≅ estadoVazio()',
  isoOperacional(estadoVazioRemoto(), estadoVazio()) &&
  estadoVazioRemoto().magia === MAGIA)

{
  const S0 = estadoVazio()
  const t = '2026-08-29T00:00:00.000Z'
  const inst = { nome: 'bash', in: 'echo ok', out: 'ok\n', t }
  const S1 = aplicaInstrucao(S0, inst.nome, inst.in, inst.out, inst.t)

  const ls = memoriaLS()
  const idb = memoriaIDB()
  const Sw = cicloWasm(S1, ls)
  const Si = cicloIDB(S1, idb)
  const Sr = cicloRemoto(S1)

  ok('§M5 M_wasm persiste GKBANCO',
    Sw.magia === MAGIA && Sw.shells.bash.out === 'ok\n' && Sw.shells.bash.in === 'echo ok')
  ok('§M5 M_remota normaliza o mesmo JSON',
    Sr.magia === MAGIA && Sr.shells.bash.out === 'ok\n')
  ok('§M5 S0+instrucao → S\' iso wasm/remota', isoOperacional(Sw, Sr))
  ok('§M5 S1_LS = S1_IDB (mesmo GKBANCO)',
    isoOperacional(Sw, Si) && Si.magia === MAGIA && Si.shells.bash.out === 'ok\n')
  ok('§M5 merge wasm/remota e idempotente',
    isoOperacional(mergeEstado(Sw, Sr), Sw) && isoOperacional(mergeEstado(Sr, Sw), Sr))

  const S2 = aplicaInstrucao(S0, inst.nome, inst.in, inst.out, inst.t)
  gravaPagina(S2, { html: '<p>x</p>', css: 'p{}', js: '1' })
  ok('§M5 pagina no mesmo S\'',
    isoOperacional(cicloWasm(S2, memoriaLS()), cicloRemoto(S2)) &&
    isoOperacional(cicloWasm(S2, memoriaLS()), cicloIDB(S2, memoriaIDB())))
}

{
  const idb = memoriaIDB()
  idb.setItem(CHAVE_ESTADO, '{"magia":"GKBANCO"}')
  idb.setItem(CHAVE_MANIFESTO, '{"linguagens":[]}')
  idb.setItem(chaveWasm('sql'), 'wasm-sql')
  const pe = parteChave(CHAVE_ESTADO)
  const pm = parteChave(CHAVE_MANIFESTO)
  const pw = parteChave(chaveWasm('sql'))
  ok('§M5b mapeamento literal das chaves',
    pe.store === STORE_ESTADO && pe.key === CHAVE_ESTADO &&
    pm.store === STORE_MANIFESTO &&
    pw.store === STORE_WASM && pw.key === 'sql')
  ok('§M5b object stores recebem o valor',
    idb._stores[STORE_ESTADO].get(CHAVE_ESTADO) === '{"magia":"GKBANCO"}' &&
    idb._stores[STORE_MANIFESTO].get(CHAVE_MANIFESTO) === '{"linguagens":[]}' &&
    idb._stores[STORE_WASM].get('sql') === 'wasm-sql')
  ok('§M5b API isomorfa le as mesmas chaves',
    idb.getItem(CHAVE_ESTADO).includes(MAGIA) &&
    leManifestoLS(idb).linguagens &&
    idb.getItem(chaveWasm('sql')) === 'wasm-sql' &&
    idb.length === 3)
}

ok('§M6 docker nao e N/A',
  nodoRealizacao(REALIZACOES.find((r) => r.id === 'docker')).estatuto === 'nao localizada' &&
  nodoRealizacao(REALIZACOES.find((r) => r.id === 'docker')).estatuto !== 'N/A')
ok('§M6 wasm+idb e mongo nao sao N/A',
  REALIZACOES.find((r) => r.id === 'wasm+idb').estatuto === 'nao localizada' &&
  REALIZACOES.find((r) => r.id === 'mongo').estatuto === 'nao localizada' &&
  REALIZACOES.find((r) => r.id === 'mongo').estatuto !== 'N/A')
ok('§M6 solo = so M_wasm', modoDaSessao({}) === 'solo')
ok('§M6 remoto = addr+pub',
  modoDaSessao({ endereco: 'ws://x/canal', chave: 'aa' }) === 'remoto')

ok('§M7 maquina minima no mvp',
  /WASM \+ memoria \+ disco/.test(man.mvp?.maquina || '') &&
  /browser realiza/.test(man.mvp?.maquina || ''))
ok('§M7 mvp M e M_min explicitos',
  man.mvp?.M === '(Exec, D, C)' &&
  man.mvp?.M_min === '(WASM, D_*, Memoria local)' &&
  /C=MOVE nao entra no iso de disco/.test(man.mvp?.maquina || ''))

{
  const def = await escolheDisco({})
  const flag = await escolheDisco({ disco: 'idb' })
  ok('§M8 boot default nao e IDB', typeof def._stores === 'undefined')
  ok('§M8 flag disco=idb devolve a ponte', flag._stores && flag.getItem)
}

{
  const S0 = estadoVazio()
  const t = '2026-08-29T00:00:00.000Z'
  const S1a = aplicaInstrucao(S0, 'bash', 'echo ok', 'ok\n', t)
  const S1b = aplicaInstrucao(S0, 'bash', 'echo ok', 'ok\n', '2020-01-01T00:00:00.000Z')
  S1a.atualizado = '2026-08-29T12:00:00.000Z'
  S1b.atualizado = '2020-01-01T00:00:00.000Z'
  const Sw = cicloWasm(S1a, memoriaLS())
  const Si = cicloIDB(JSON.parse(JSON.stringify(S1a)), memoriaIDB())
  ok('§M9 hashCanonico ignora t/atualizado',
    hashCanonico(S1a) === hashCanonico(S1b) &&
    !hashCanonico.toString().includes('Date') &&
    !estadoCanonico.toString().includes('Date'))
  ok('§M9 hash S1_LS = S1_IDB',
    hashCanonico(Sw) === hashCanonico(Si) &&
    hashCanonico(Sw) === hashCanonico(S1a) &&
    isoOperacional(Sw, Si))
  ok('§M9 hash vazio e deterministico',
    hashCanonico(estadoVazio()) === hashCanonico(estadoVazioRemoto()) &&
    hashCanonico(estadoVazio()).length === 8)
}

{
  const seq = [
    [CHAVE_ESTADO, '{"magia":"GKBANCO","v":2}'],
    [CHAVE_MANIFESTO, '{"linguagens":[]}'],
    [chaveWasm('sql'), 'wasm-sql'],
  ]
  const ls = memoriaLS()
  const idb = memoriaIDB()
  for (const [k, v] of seq) {
    ls.setItem(k, v)
    idb.setItem(k, v)
  }
  const ol = ordemVisibilidade(ls)
  const oi = ordemVisibilidade(idb)
  ok('§M10 ordem de visibilidade LS = IDB',
    ol.length === seq.length && oi.length === seq.length &&
    ol.every((p, i) => p[0] === oi[i][0] && p[1] === oi[i][1]) &&
    ol.every((p, i) => p[0] === seq[i][0] && p[1] === seq[i][1]))
}

{
  function quotaLS (limite) {
    const m = new Map()
    return {
      getItem (k) { return m.has(k) ? m.get(k) : null },
      setItem (k, v) {
        const n = 2 * (String(k).length + String(v).length)
        if (n > limite) {
          const err = new Error('quota')
          err.name = 'QuotaExceededError'
          throw err
        }
        m.set(String(k), String(v))
      },
      removeItem (k) { m.delete(String(k)) },
      key (i) { return [...m.keys()][i] ?? null },
      get length () { return m.size },
    }
  }
  const anterior = '{"magia":"GKBANCO","v":2,"shells":{},"atualizado":null,"pendente":[]}'
  const ls = quotaLS(80)
  ls.setItem(CHAVE_ESTADO, 'ok')
  let lsFalhou = false
  try { ls.setItem(CHAVE_ESTADO, 'x'.repeat(200)) } catch (e) { lsFalhou = e.name === 'QuotaExceededError' }
  const idb = memoriaIDB()
  idb.setItem(CHAVE_ESTADO, anterior)
  idb.abortar()
  let idbFalhou = false
  try { idb.setItem(CHAVE_ESTADO, '{"magia":"NOVO"}') } catch (e) { idbFalhou = e.name === 'AbortError' }
  ok('§M11 falha parcial: API deixa o valor anterior',
    lsFalhou && ls.getItem(CHAVE_ESTADO) === 'ok' &&
    idbFalhou && idb.getItem(CHAVE_ESTADO) === anterior &&
    idb._stores[STORE_ESTADO].get(CHAVE_ESTADO) === anterior)
  const idbNodo = DISCOS.find((d) => d.id === 'IndexedDB')
  ok('§M11 mapa QuotaExceeded↔abort no nodo IndexedDB',
    /QuotaExceeded/.test(idbNodo?.evidencia || '') &&
    /abort/.test(idbNodo?.evidencia || '') &&
    /corte\/retenta so no LS/.test(idbNodo?.evidencia || '') &&
    idbNodo?.estatuto === 'realizado')
  ok('§M11 wasm+idb continua nao localizada',
    REALIZACOES.find((r) => r.id === 'wasm+idb')?.estatuto === 'nao localizada')
}

ok('§M4b canal invariante: offsets e JSON vazio do C',
  SLOTS_ESTADO.in === 9210 && SLOTS_ESTADO.out === 9211 &&
  S_ESTADO_REQ === S_CANAL + 9210 && S_ESTADO_RSP === S_CANAL + 9211 &&
  (patria.includes(ESTADO_VAZIO_REMOTO.replace(/"/g, '\\"')) ||
    patria.includes(ESTADO_VAZIO_REMOTO)))

{
  const S0 = estadoVazio()
  const t = '2026-08-29T00:00:00.000Z'
  const S1 = aplicaInstrucao(S0, 'bash', 'echo ok', 'ok\n', t)
  const ls = memoriaLS()
  const idb = memoriaIDB()
  const cLS = medeIO(S1, ls, 32)
  const cIDB = medeIO(S1, idb, 32)
  const custo = {
    ls: { n: cLS.n, grava_ms: cLS.grava_ms, le_ms: cLS.le_ms, bytes: cLS.bytes },
    idb: { n: cIDB.n, grava_ms: cIDB.grava_ms, le_ms: cIDB.le_ms, bytes: cIDB.bytes },
    nota: 'custo != correcao; shim IDB; sem promocao wasm+idb',
  }
  console.log('#CUSTO ' + JSON.stringify(custo))
  ok('§M12 custo IO e numero (nao criterio de iso)',
    Number.isFinite(cLS.grava_ms) && Number.isFinite(cLS.le_ms) &&
    Number.isFinite(cIDB.grava_ms) && Number.isFinite(cIDB.le_ms) &&
    cLS.n === 32 && cIDB.bytes === cLS.bytes && cLS.bytes > 0)
  ok('§M12 apos medeIO ainda iso LS/IDB',
    isoOperacional(leEstado(ls), leEstado(idb)) &&
    hashCanonico(leEstado(ls)) === hashCanonico(leEstado(idb)))
  ok('§M12 wasm+idb nao promovida pelo custo',
    REALIZACOES.find((r) => r.id === 'wasm+idb')?.estatuto === 'nao localizada')
}

{
  const S0 = estadoVazio()
  const t = '2026-08-29T00:00:00.000Z'
  const S1 = aplicaInstrucao(S0, 'bash', 'echo ok', 'ok\n', t)
  const oraculo = hashCanonico(S1)
  const Sw = cicloWasm(S1, memoriaLS())
  const raiz = join(tmpdir(), 'gkban-vol-' + process.pid)
  const Sv = cicloVolume(S1, {
    escrever (rel, s) {
      const p = join(raiz, ...String(rel).split('/'))
      mkdirSync(dirname(p), { recursive: true })
      writeFileSync(p, s)
    },
    ler (rel) { return readFileSync(join(raiz, ...String(rel).split('/')), 'utf8') },
  })
  ok('§M13 oraculo hash S0+I', oraculo === hashCanonico(S1) && oraculo.length === 8)
  ok('§M13 par WASM/volume mesmo hash',
    hashCanonico(Sw) === oraculo && hashCanonico(Sv) === oraculo &&
    isoOperacional(Sw, Sv))
  ok('§M13 ficheiro de volume e gk/banco/estado.json',
    VOLUME_ESTADO === 'gk/banco/estado.json' &&
    existsSync(join(raiz, ...VOLUME_ESTADO.split('/'))))
  try { rmSync(raiz, { recursive: true, force: true }) } catch { /* tmp */ }

  const docker = REALIZACOES.find((r) => r.id === 'docker')
  const df = docker?.fonte ? readFileSync(join(RAIZ, docker.fonte), 'utf8') : ''
  ok('§M13 M_Docker Exec=docker D=volume C=canal',
    docker?.slots?.Exec === 'docker' && docker?.slots?.D === 'volume' &&
    docker?.slots?.C === 'canal' && docker?.slots?.C === SLOTS_M.C)
  ok('§M13 M_Docker nao localizada (sem paridade de contentor)',
    docker?.estatuto === 'nao localizada' && docker?.estatuto !== 'N/A' &&
    /sem paridade de contentor/.test(docker?.evidencia || ''))
  ok('§M13 Dockerfile.volume objecto de ingestao',
    docker?.fonte === 'banco/Dockerfile.volume' &&
    existsSync(join(RAIZ, docker.fonte)) &&
    /VOLUME \/gk\/banco/.test(df) &&
    /FROM alpine/.test(df) &&
    !/docker-compose/i.test(df))
  ok('§M13 sem cold-start inventado',
    !/\d+\s*ms/.test(docker?.evidencia || '') &&
    /sem cold-start inventado/.test(docker?.evidencia || ''))
  ok('§M13 C invariante 9210/9211',
    SLOTS_ESTADO.in === 9210 && SLOTS_ESTADO.out === 9211 &&
    S_ESTADO_REQ === S_CANAL + 9210 && S_ESTADO_RSP === S_CANAL + 9211)
  ok('§M13 mongo continua nao localizada',
    REALIZACOES.find((r) => r.id === 'mongo')?.estatuto === 'nao localizada' &&
    REALIZACOES.find((r) => r.id === 'mongo')?.estatuto !== 'N/A')
  ok('§M13 troca: D antes de Exec (fis:thm:troca-realizacao)',
    /fis:thm:troca-realizacao/.test(docker?.evidencia || '') &&
    /fis:def:objeto/.test(docker?.evidencia || '') &&
    /fis:thm:troca-realizacao/.test(DISCOS.find((d) => d.id === 'volume')?.evidencia || ''))
}

{
  const S0 = estadoVazio()
  const t = '2026-08-29T00:00:00.000Z'
  const wasmPath = join(RAIZ, 'assets', 'figuras', 'wasm', 'consultar.wasm')
  const q = "INSERT TEXTO 'faseb'"
  let I = { nome: 'sql', in: q, out: '', t }
  if (existsSync(wasmPath)) {
    const tags = instrucaoSqlWasm(readFileSync(wasmPath), q)
    I = { nome: 'sql', in: q, out: tags, t }
  }
  const S1 = aplicaInstrucao(S0, I.nome, I.in, I.out, I.t)
  const oraculo = hashCanonico(S1)
  const pub = '00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff'
  const idAntes = resolveIdentidade({ chave: pub })

  const Sw = cicloWasm(S1, memoriaLS())
  const raiz = join(tmpdir(), 'gkban-exec-' + process.pid)
  function ioVol (base) {
    return {
      escrever (rel, s) {
        const p = join(base, ...String(rel).split('/'))
        mkdirSync(dirname(p), { recursive: true })
        writeFileSync(p, s)
      },
      ler (rel) { return readFileSync(join(base, ...String(rel).split('/')), 'utf8') },
    }
  }
  const Sv = cicloExecNativo(S1, ioVol(raiz))
  const volHost = join(raiz, 'gk', 'banco')
  const sonda = sondaDocker(spawnSync)
  const dock = cicloDocker(S1, { spawnSync, volumeHost: volHost, sonda })
  const idDepois = resolveIdentidade({ chave: pub })
  const docker = REALIZACOES.find((r) => r.id === 'docker')
  const df = docker?.fonte ? readFileSync(join(RAIZ, docker.fonte), 'utf8') : ''
  const dfAtivo = df.split('\n').filter((l) => !/^\s*#/.test(l)).join('\n')

  ok('§M14 I=sql_move em consultar.wasm (sem rede)',
    existsSync(wasmPath) && I.out.length > 0 && I.out.includes('<') &&
    NULO_ARENA === 8)
  ok('§M14 hash(S1_wasm)=hash(S1_volume) oraculo',
    hashCanonico(Sw) === oraculo && hashCanonico(Sv) === oraculo &&
    isoOperacional(Sw, Sv))
  if (dock.correu) {
    ok('§M14 hash(S1_docker)=hash(S1_wasm)',
      hashCanonico(dock.estado) === oraculo && isoOperacional(dock.estado, Sw))
  } else {
    ok('§M14 M_Docker nao localizada (sem contentor medido)',
      dock.estatuto === 'nao localizada' && dock.estado === null &&
      docker?.estatuto === 'nao localizada' && docker?.estatuto !== 'N/A')
  }
  ok('§M14 identidade intocada no ciclo Exec',
    idAntes.camada === 'chave' &&
    JSON.stringify(idAntes) === JSON.stringify(idDepois) &&
    JSON.stringify(idAntes) === JSON.stringify(resolveIdentidade({ chave: pub })) &&
    docker?.id === 'docker' &&
    !('id' in (docker?.slots || {})) &&
    /Identidade intocada/.test(docker?.evidencia || '') &&
    /nenhum nodo redefine id/.test(docker?.evidencia || '') &&
    /nao redefine id/.test(docker?.proibicao || ''))
  ok('§M14 S_DEPOSITO 9220/9221 ≠ S_ESTADO 9210/9211',
    S_DEPOSITO_REQ === S_CANAL + 9220 && S_DEPOSITO_RSP === S_CANAL + 9221 &&
    S_ESTADO_REQ === S_CANAL + 9210 && S_ESTADO_RSP === S_CANAL + 9211 &&
    S_DEPOSITO_REQ !== S_ESTADO_REQ && S_DEPOSITO_RSP !== S_ESTADO_RSP &&
    !/deposito\.bin/i.test(df) &&
    /S_DEPOSITO 9220\/9221 != S_ESTADO/.test(docker?.evidencia || ''))
  ok('§M14 Dockerfile.volume sem privileged / host network',
    !/privileged/i.test(dfAtivo) &&
    !/--network/i.test(dfAtivo) &&
    !/network\s*=\s*host|--network\s+host/i.test(dfAtivo) &&
    !/deposito\.bin/i.test(df) &&
    /VOLUME \/gk\/banco/.test(df))
  ok('§M14 wasm+idb e mongo nao promovidos',
    REALIZACOES.find((r) => r.id === 'wasm+idb')?.estatuto === 'nao localizada' &&
    REALIZACOES.find((r) => r.id === 'mongo')?.estatuto === 'nao localizada')
  ok('§M14 Fase B nao toca S_DEPOSITO no Exec',
    !cicloVolume.toString().includes('deposito') &&
    !cicloExecNativo.toString().includes('9220') &&
    !instrucaoSqlWasm.toString().includes('9220') &&
    !cicloDocker.toString().includes('deposito.bin'))
  try { rmSync(raiz, { recursive: true, force: true }) } catch { /* tmp */ }
}

console.log('')
if (!falhas) {
  console.log('  Fase A: iso+hash+ordem + custo IO (custo != correcao). wasm+idb nao localizada.')
  console.log('  Fase B Exec: wasm=volume no oraculo; M_Docker so se contentor mediou; Identidade intocada.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
