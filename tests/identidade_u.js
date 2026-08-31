/* tests/identidade_u.js — identidade operacional: chave > sessão > fingerprint > oauth.
 *
 * Identidade ≠ realização da máquina (Exec/D/C). Sem segundo $id.
 *   node tests/identidade_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { createHash } from 'node:crypto'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import { memoriaIDB, parteChave, STORE_KV } from '../app/src/banco_disco.js'
import {
  SLOTS_IDENTIDADE, SLOTS_CICLO, SLOTS_DEPOSITO, FORCA, CHAVE_SESSAO, NATUREZA_BIND,
  tabelaSlots, tabelaCiclo,
  resolveIdentidade, idEstavelDaChave, hexBanda,
  ligaSessao, leRegistoSessao, bind, ligaIdentidade,
  sinaisDoAmbiente, hashFingerprint,
  identidadeParaU, uParaIdentidade,
  bootstrapRealizacao, importaPub, rodaIdentidade, protocoloRecuperacao, novoSujeito,
  oauthDefineId, idPorOAuth, eJsonGKBANCO,
  bandaDeId, selectorDoId, validaBanda, F, supp, chi, caractere,
} from '../app/src/banco_identidade_u.js'
import { TRANSF_N, norma2, residuoParseval, bandasDisjuntas } from '../app/src/banco_transf_u.js'
import { igual } from '../app/src/banco_manifesto_u.js'
import { REALIZACOES } from '../app/src/banco_maquina_u.js'
import { bandaDeChave, selaBlob, abreBlob } from '../tools/banco_banda.mjs'
import { S_CANAL, S_ESTADO_REQ, S_DEPOSITO_REQ, S_DEPOSITO_RSP } from '../tools/canal_slots.mjs'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const man = JSON.parse(readFileSync(MAN, 'utf8'))
const schema = JSON.parse(readFileSync(SCHEMA, 'utf8'))
const pub = '00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff'
const uuid = '11111111-2222-4333-8444-555555555555'

ok('§I0 ponte_identidade no motor',
  man.corpos?.motor?.ponte_identidade === 'app/src/banco_identidade_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_identidade)))
ok('§I0 schema $id tiffany://u (sem kind identidade)',
  schema.$id === 'tiffany://u' &&
  !schema.properties.kind.enum.includes('identidade'))
ok('§I0 identidade nao e realizacao da maquina',
  REALIZACOES.every((r) => r.id !== 'identidade' && r.id !== 'chave' && r.id !== 'sessao'))

const tab = tabelaSlots()
ok('§I0 tabela: chave realizado, sessao realizado',
  tab.find((s) => s.id === 'chave')?.estatuto === 'realizado' &&
  tab.find((s) => s.id === 'sessao')?.estatuto === 'realizado')
ok('§I0 tabela: fingerprint candidato, oauth nao localizada',
  tab.find((s) => s.id === 'fingerprint')?.estatuto === 'candidato' &&
  tab.find((s) => s.id === 'oauth')?.estatuto === 'nao localizada')
ok('§I0 oauth nao localizada ≠ N/A',
  SLOTS_IDENTIDADE.find((s) => s.id === 'oauth')?.estatuto === 'nao localizada' &&
  SLOTS_IDENTIDADE.find((s) => s.id === 'oauth')?.estatuto !== 'N/A' &&
  /nao e N\/A/.test(SLOTS_IDENTIDADE.find((s) => s.id === 'oauth')?.evidencia || ''))
ok('§I0 forca chave > sessao > fingerprint > oauth',
  FORCA.chave > FORCA.sessao && FORCA.sessao > FORCA.fingerprint && FORCA.fingerprint > FORCA.oauth)

{
  const a = resolveIdentidade({ chave: pub, sessao: uuid, fingerprint: 'abc', oauth: 'google' })
  const b = resolveIdentidade({ oauth: 'google', fingerprint: 'abc', sessao: uuid, chave: pub })
  ok('§I1 chave vence sessao', a.camada === 'chave' && a.chave === pub && a.forca === FORCA.chave)
  ok('§I1 resolucao deterministica (ordem das evidencias irrelevante)',
    a.camada === b.camada && a.chave === b.chave && a.sessao === b.sessao && a.forca === b.forca)
  ok('§I1 fingerprint nao substitui chave',
    resolveIdentidade({ chave: pub, fingerprint: 'fff' }).camada === 'chave')
  ok('§I1 oauth nao e usado',
    resolveIdentidade({ oauth: 'google' }).camada === null &&
    resolveIdentidade({ oauth: 'google', sessao: uuid }).camada === 'sessao')
  ok('§I1 sem chave cai na sessao',
    resolveIdentidade({ sessao: uuid, fingerprint: 'fff' }).camada === 'sessao')
  ok('§I1 chave maiuscula = mesma resolucao',
    resolveIdentidade({ chave: pub.toUpperCase() }).chave === pub)
}

{
  const id = await idEstavelDaChave(pub)
  const banda = bandaDeChave(pub)
  ok('§I2 id da chave = hex da banda sha256(bytes), nao utf8 do hex',
    id === banda.toString('hex') &&
    id === createHash('sha256').update(Buffer.from(pub, 'hex')).digest('hex') &&
    id !== createHash('sha256').update(pub, 'utf8').digest('hex'))
  ok('§I2 mesma chave → mesmo id (atravessa realizacoes)',
    (await idEstavelDaChave(pub.toUpperCase())) === id &&
    hexBanda(banda).length === 64)
}

{
  const st = memoriaLS()
  const u1 = ligaSessao(st)
  const u2 = ligaSessao(st)
  ok('§I3 UUID persistido gk:banco:sessao',
    u1 && u1 === u2 && leRegistoSessao(st).uuid === u1 && st.getItem(CHAVE_SESSAO))
  const lig = bind(u1, pub, st, { t: '2026-08-29T00:00:00.000Z' })
  ok('§I3 bind e explicito (t, de, para)',
    lig.t === '2026-08-29T00:00:00.000Z' && lig.de === u1 && lig.para === pub)
  ok('§I3 bind marcado soft no manifesto e no registo',
    lig.natureza === NATUREZA_BIND && NATUREZA_BIND === 'soft' &&
    /soft/.test(man.mvp?.identidade?.ciclo?.bind || man.mvp?.identidade?.slots?.sessao || ''))
  const binds = leRegistoSessao(st).binds
  ok('§I3 bind gravado no disco, nao «parece o mesmo»',
    binds.length === 1 && binds[0].de === u1 && binds[0].para === pub && binds[0].t === lig.t &&
    binds[0].natureza === 'soft')
  let recusou = false
  try { bind('', pub, st) } catch { recusou = true }
  ok('§I3 bind sem sessao recusa', recusou)
}

{
  const st = memoriaLS()
  const semPub = await ligaIdentidade(st, {})
  ok('§I4 sem pub → sessao local',
    semPub.camada === 'sessao' && semPub.uuid && semPub.bind === null)
  const comPub = await ligaIdentidade(st, { chave: pub })
  ok('§I4 com pub apos sessao → chave + bind',
    comPub.camada === 'chave' && comPub.bind && comPub.bind.de === semPub.uuid && comPub.bind.para === pub)
  const soChave = await ligaIdentidade(memoriaLS(), { chave: pub })
  ok('§I4 primeira visita com pub: identidade=chave, sem bind (nao havia sessao)',
    soChave.camada === 'chave' && soChave.bind === null)
}

{
  const sinais = { hardwareConcurrency: 8, language: 'pt-BR', timezone: 'America/Sao_Paulo' }
  const h1 = await hashFingerprint(sinais)
  const h2 = await hashFingerprint({ timezone: 'America/Sao_Paulo', hardwareConcurrency: 8, language: 'pt-BR' })
  ok('§I5 fingerprint hash estavel (chaves ordenadas)', h1 === h2 && h1.length === 64)
  const amb = sinaisDoAmbiente({ navigator: { hardwareConcurrency: 4, language: 'pt' } })
  ok('§I5 sinais: concurrency+idioma; sem user-agent',
    amb.hardwareConcurrency === 4 && amb.language === 'pt' && !('userAgent' in amb) && !('width' in amb))
  ok('§I5 fingerprint nunca e camada vencedora',
    resolveIdentidade({ fingerprint: h1, chave: pub }).camada === 'chave' &&
    resolveIdentidade({ fingerprint: h1, sessao: uuid }).camada === 'sessao' &&
    resolveIdentidade({ fingerprint: h1 }).camada === null)
}

{
  const U = identidadeParaU(resolveIdentidade({ chave: pub, sessao: uuid }), { modo: 'solo' })
  ok('§I6 kind=sessao id=identidade, nao realizacao',
    U.kind === 'sessao' && U.id === 'identidade' && U.star !== 'D')
  ok('§I6 faces MOVE ±1', U.faces.menos.sentido === -1 && U.faces.mais.sentido === 1)
  ok('§I6 slots do censo no nodo',
    U.slots.chave === 'realizado' &&
    U.slots.sessao === 'realizado' &&
    U.slots.fingerprint === 'candidato' &&
    U.slots.oauth === 'nao localizada')
  ok('§I6 filho fingerprint = nao localizada + nota candidato (enum U)',
    U.filhos.find((f) => f.id === 'fingerprint')?.estatuto === 'nao localizada' &&
    U.filhos.find((f) => f.id === 'fingerprint')?.nota === 'candidato')
  ok('§I6 filho oauth = nao localizada ≠ N/A',
    U.filhos.find((f) => f.id === 'oauth')?.estatuto === 'nao localizada' &&
    U.filhos.find((f) => f.id === 'oauth')?.estatuto !== 'N/A')
  ok('§I6 U→projecao guarda chave', uParaIdentidade(U).chave === pub)

  const extra = []
  function walk (n) {
    for (const k of Object.keys(n || {})) {
      if (!(k in schema.properties)) extra.push((n.kind || '?') + '.' + k)
    }
    if (n?.faces?.menos) walk(n.faces.menos)
    if (n?.faces?.mais) walk(n.faces.mais)
    for (const f of n?.filhos || []) walk(f)
  }
  walk(U)
  ok('§I6 identidade cabe no schema U', extra.length === 0)
}

{
  const idb = memoriaIDB()
  const uuidIdb = ligaSessao(idb)
  bind(uuidIdb, pub, idb, { t: '2026-08-29T12:00:00.000Z' })
  ok('§I7 mesma ponte IDB (gk:banco:sessao → STORE_KV)',
    parteChave(CHAVE_SESSAO).store === STORE_KV &&
    leRegistoSessao(idb).uuid === uuidIdb &&
    leRegistoSessao(idb).binds.length === 1)
  const idLs = await idEstavelDaChave(pub)
  const idIdb = await idEstavelDaChave(pub)
  ok('§I7 chave atravessa LS/IDB: mesmo id, sem depender do disco',
    idLs === idIdb && igual(idLs, idIdb))
}

{
  const id = man.mvp?.identidade
  ok('§I8 censo no manifesto: ponte + slots',
    id?.ponte === 'app/src/banco_identidade_u.js' &&
    /realizado/.test(id?.slots?.chave || '') &&
    /realizado/.test(id?.slots?.sessao || '') &&
    /candidato/.test(id?.slots?.fingerprint || '') &&
    /nao localizada/.test(id?.slots?.oauth || '') &&
    /nao e N\/A/.test(id?.slots?.oauth || ''))
  ok('§I8 identidade != maquina no mvp',
    /identidade != Exec/.test(id?.nota || '') || /!= Exec/.test(id?.proibicao || ''))
}

{
  const cic = tabelaCiclo()
  const ids = cic.map((s) => s.id).join(',')
  ok('§I9 ciclo: bootstrap/bind/rotacao/recuperacao/privada_wasm/privada_docker',
    ids === 'bootstrap,bind,rotacao,recuperacao,privada_wasm,privada_docker')
  ok('§I9 bootstrap e bind realizados; bind soft',
    cic.find((s) => s.id === 'bootstrap')?.estatuto === 'realizado' &&
    cic.find((s) => s.id === 'bind')?.estatuto === 'realizado' &&
    cic.find((s) => s.id === 'bind')?.nota === 'soft' &&
    /soft/.test(cic.find((s) => s.id === 'bind')?.evidencia || ''))
  ok('§I9 rotacao candidato; recuperacao nao localizada ≠ N/A',
    cic.find((s) => s.id === 'rotacao')?.estatuto === 'candidato' &&
    SLOTS_CICLO.find((s) => s.id === 'recuperacao')?.estatuto === 'nao localizada' &&
    SLOTS_CICLO.find((s) => s.id === 'recuperacao')?.estatuto !== 'N/A')
  ok('§I9 privada_wasm candidato; privada_docker nao localizada',
    cic.find((s) => s.id === 'privada_wasm')?.estatuto === 'candidato' &&
    SLOTS_CICLO.find((s) => s.id === 'privada_docker')?.estatuto === 'nao localizada')
  ok('§I9 ciclo no manifesto',
    /realizado/.test(man.mvp?.identidade?.ciclo?.bootstrap || '') &&
    /soft/.test(man.mvp?.identidade?.ciclo?.bind || '') &&
    /candidato/.test(man.mvp?.identidade?.ciclo?.rotacao || '') &&
    /nao localizada/.test(man.mvp?.identidade?.ciclo?.recuperacao || '') &&
    /candidato/.test(man.mvp?.identidade?.ciclo?.privada_wasm || '') &&
    /nao localizada/.test(man.mvp?.identidade?.ciclo?.privada_docker || ''))
  const U = identidadeParaU(resolveIdentidade({ chave: pub }))
  ok('§I9 filhos ciclo no U; bind filho = realizado + nota soft',
    U.slots.bootstrap === 'realizado' &&
    U.filhos.find((f) => f.id === 'bind')?.estatuto === 'realizado' &&
    U.filhos.find((f) => f.id === 'bind')?.nota === 'soft' &&
    U.filhos.find((f) => f.id === 'rotacao')?.nota === 'candidato' &&
    U.filhos.find((f) => f.id === 'recuperacao')?.estatuto === 'nao localizada')
}

{
  const st = memoriaLS()
  const boot = await bootstrapRealizacao(st, { pub })
  ok('§I10 bootstrap ?pub= → chave; primeira visita sem bind',
    boot.camada === 'chave' && boot.bind === null && boot.id === await idEstavelDaChave(pub))
  const st2 = memoriaLS()
  ligaSessao(st2)
  const imp = await importaPub(st2, pub)
  ok('§I10 importacao apos UUID → chave + bind soft',
    imp.camada === 'chave' && imp.bind && imp.bind.natureza === 'soft')
  const idOld = await idEstavelDaChave(pub)
  const pub2 = 'ffeeddccbbaa99887766554433221100ffeeddccbbaa99887766554433221100'
  const idNew = await idEstavelDaChave(pub2)
  const rot = rodaIdentidade(idOld, idNew, st, { t: '2026-08-29T18:00:00.000Z' })
  ok('§I10 rotacao auditavel {t,de,para} sem prova',
    rot.t === '2026-08-29T18:00:00.000Z' && rot.de === idOld && rot.para === idNew &&
    leRegistoSessao(st).rotacoes.length === 1)
  ok('§I10 rotacao nao muda o sujeito sem nova pub',
    (await ligaIdentidade(st, { chave: pub })).id === idOld && idOld !== idNew)
  const rec = protocoloRecuperacao()
  ok('§I10 recuperacao nao localizada; sem shares; terceiros nao redefinem',
    rec.estatuto === 'nao localizada' && rec.estatuto !== 'N/A' &&
    /shares/.test(rec.evidencia) && /nao redefinem/.test(rec.evidencia))
  const novo = await novoSujeito(memoriaLS(), pub2)
  ok('§I10 novo sujeito = nova chave; oauth nao entra',
    novo.id === idNew && novo.id !== idOld && !oauthDefineId() && idPorOAuth('google') === null)
}

{
  const banda = bandaDeChave(pub)
  const claro = Buffer.from('{"magia":"GKBANCO","v":2,"shells":{},"atualizado":null,"pendente":[]}')
  const selado = selaBlob(claro, banda)
  ok('§I11 claro e GKBANCO; selado nao e JSON parseavel',
    eJsonGKBANCO(claro) && !eJsonGKBANCO(selado) &&
    Buffer.compare(abreBlob(selado, banda), claro) === 0)
  ok('§I11 S_DEPOSITO 9220/9221 ≠ S_ESTADO 9210; disco deposito.bin',
    SLOTS_DEPOSITO.in === 9220 && SLOTS_DEPOSITO.out === 9221 &&
    S_DEPOSITO_REQ === S_CANAL + 9220 && S_DEPOSITO_RSP === S_CANAL + 9221 &&
    S_ESTADO_REQ === S_CANAL + 9210 &&
    SLOTS_DEPOSITO.disco === 'gk/banco/deposito.bin' &&
    /9220/.test(man.mvp?.identidade?.deposito || '') &&
    /deposito\.bin/.test(man.mvp?.armazenamento?.deposito || ''))
  const sql = readFileSync(join(RAIZ, 'banco', 'sql.c'), 'utf8')
  const pat = readFileSync(join(RAIZ, 'banco', 'canal_patria.c'), 'utf8')
  ok('§I11 sql.c e canal_patria documentam S_DEPOSITO opaco',
    sql.includes('#define S_DEPOSITO_REQ (S_CANAL + 9220u)') &&
    sql.includes('#define S_DEPOSITO_RSP (S_CANAL + 9221u)') &&
    pat.includes('deposito.bin') && pat.includes('S_DEPOSITO_REQ') &&
    !/GKBANCO/.test((pat.match(/serve_deposito[\s\S]*?^static void serve_estado/m) || [''])[0]))
  ok('§I11 banda do canal = id (sem gk:id:v1)',
    (await idEstavelDaChave(pub)) === banda.toString('hex') &&
    /sem prefixo gk:id:v1/.test(man.mvp?.identidade?.nota || ''))
}

{
  const id = await idEstavelDaChave(pub)
  const comOauth = resolveIdentidade({ chave: pub, oauth: 'google', privada_wasm: 'idb', privada_docker: '/sec' })
  const soOauth = resolveIdentidade({ oauth: 'apple' })
  ok('§I12 oauth nao redefine id',
    comOauth.camada === 'chave' && comOauth.chave === pub &&
    soOauth.camada === null && idPorOAuth({ vendor: 'google', sub: 'x' }) === null)
  ok('§I12 privada wasm/docker nao vazam para resolveIdentidade',
    !('privada_wasm' in comOauth) && !('privada_docker' in comOauth) &&
    !('wasm' in comOauth) && !('docker' in comOauth) &&
    JSON.stringify(comOauth) === JSON.stringify(resolveIdentidade({ chave: pub })))
  ok('§I12 id operacional ignora oauth',
    (await ligaIdentidade(memoriaLS(), { chave: pub, oauth: 'google' })).id === id)
}

{
  const pub2 = 'ffeeddccbbaa99887766554433221100ffeeddccbbaa99887766554433221100'
  const id1 = await idEstavelDaChave(pub)
  const id2 = await idEstavelDaChave(pub2)
  const B1 = bandaDeId(id1)
  const B2 = bandaDeId(id2)
  const s1 = selectorDoId(id1)
  const s2 = selectorDoId(id2)
  ok('§I13 dois id ⇒ B(id) disjuntos no mesmo S',
    id1 !== id2 && s1 !== s2 && bandasDisjuntas(id1, id2) &&
    B1.length === 1 && B2.length === 1 && B1[0] === s1 && B2[0] === s2)
  let prod = 0
  const c1 = caractere(s1)
  const c2 = caractere(s2)
  for (let x = 0; x < TRANSF_N; x++) prod += c1[x] * c2[x]
  ok('§I13 bandas ortogonais (caracteres χ_s)',
    prod === 0 && s1 !== s2)
  const Ifora = caractere((s1 + 1) & (TRANSF_N - 1))
  const hatFora = F(Ifora)
  ok('§I13 suporte fora de B(id) rejeitado',
    !validaBanda(Ifora, id1) &&
    supp(hatFora).some((k) => k !== s1) &&
    !supp(hatFora).every((k) => B1.includes(k)))
  const Identro = caractere(s1)
  const hatIn = F(Identro)
  ok('§I13 suporte em B(id) aceite; Parseval residuo 0',
    validaBanda(Identro, id1) === true &&
    supp(hatIn).join(',') === String(s1) &&
    hatIn[s1] === TRANSF_N &&
    residuoParseval(Identro, hatIn) === 0 &&
    norma2(hatIn) === TRANSF_N * norma2(Identro))
  ok('§I13 id selecciona, nao e espectro (hash ≠ F(I))',
    id1 !== hatIn.join(',') &&
    id1 !== String(hatIn[s1]) &&
    id1.length === 64 &&
    selectorDoId(id1) === s1 &&
    s1 !== hatIn[s1])
  ok('§I13 chi e o da Algebra; id nao e F',
    chi(s1, 0) === 1 &&
    F(Identro)[s1] === TRANSF_N &&
    id1 !== JSON.stringify(hatIn))
  ok('§I13 Docker nao promovido',
    REALIZACOES.find((r) => r.id === 'docker')?.estatuto === 'nao localizada' &&
    REALIZACOES.find((r) => r.id === 'docker')?.estatuto !== 'realizado')
}

console.log('')
if (!falhas) {
  console.log('  Identidade: chave vence; bind soft; ciclo de vida; deposito opaco; oauth nao define id.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
