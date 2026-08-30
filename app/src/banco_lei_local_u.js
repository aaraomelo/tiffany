// banco_lei_local_u.js — varredura de candidatos de Lei Local.
// L_S = fibra da face (fis:thm:tecidos (1)) lida em Contr(C) no mesmo S.
// Não sobe a escada. Não promove. Não inventa corpo. Sem Ficha 11.
// U consome o construído; coincidência não declarada nasce nao localizada.

import { memoriaLS } from './corpo_disco.js'
import { SLOTS_DEPOSITO, eJsonGKBANCO } from './banco_identidade_u.js'
import {
  REALIZACOES,
  SLOTS_ESTADO,
  estadoVazio,
  aplicaInstrucao,
  hashCanonico,
  isoOperacional,
  cicloWasm,
  cicloRemoto,
  sondaDocker,
  cicloDocker,
} from './banco_maquina_u.js'
import { medeTransformada, validaBanda } from './banco_transf_u.js'

export { validaBanda }

export const ALVOS = Object.freeze(['S_ESTADO', 'S_DEPOSITO', 'M_WASM_M_Docker'])

export const RECUSAS = Object.freeze([
  'Ficha 11',
  'corpo novo',
  'Lei 8',
  'promocao por analogia',
  'X_{k+1} da escada',
  'fis:thm:handshake (homonimo)',
  'cone-espiral',
  'M_WASM+IDB',
  'Mongo',
  'banda canal = B_cliente',
  'FFT inventada',
  'L_S^com como corpo',
])

const T_FIXO = '2026-08-29T00:00:00.000Z'

/** Corpos já ingeridos com estatuto realizado e canónico (resíduo 0 no censo). */
export function corposResiduoZero (man) {
  return ((man && man.corpos && man.corpos.lista) || []).filter((c) =>
    c && c.estatuto === 'realizado' && c.canonico != null && c.camada === 'Fisica')
}

/** Realizações com estatuto realizado. Docker/IDB/mongo ficam de fora. */
export function realizacoesResiduoZero () {
  return REALIZACOES.filter((r) => r.estatuto === 'realizado')
}

function candidato (id, suporte, contratos, invariante, evidencia, incidencia) {
  return {
    id,
    kind: 'realizacao',
    suporte,
    contratos: contratos.slice(),
    invariante,
    evidencia,
    incidencia,
    estatuto: 'nao localizada',
    nota: 'candidato de Lei Local',
    proibicao: 'nao promover; sem Ficha 11; sem corpo novo; sem Lei 8',
    fonte: 'univ:def:lei-local-canonica',
  }
}

function medeEstado () {
  const S0 = estadoVazio()
  const S1 = aplicaInstrucao(S0, 'bash', 'echo ok', 'ok\n', T_FIXO)
  const Sw = cicloWasm(S1, memoriaLS())
  const Sr = cicloRemoto(S1)
  const hw = hashCanonico(Sw)
  const hr = hashCanonico(Sr)
  return {
    hash_wasm: hw,
    hash_remota: hr,
    iguais: hw === hr && isoOperacional(Sw, Sr),
    slots: { in: SLOTS_ESTADO.in, out: SLOTS_ESTADO.out },
  }
}

function medeDeposito (man) {
  const dep = (man && man.mvp && man.mvp.identidade && man.mvp.identidade.deposito) || ''
  const arm = (man && man.mvp && man.mvp.armazenamento && man.mvp.armazenamento.deposito) || ''
  const texto = dep + ' ' + arm
  const opaco = /opaco|blob|sem a banda|nao e JSON/i.test(texto)
  const slots = SLOTS_DEPOSITO.in === 9220 && SLOTS_DEPOSITO.out === 9221
  const naoEstado = SLOTS_DEPOSITO.in !== SLOTS_ESTADO.in
  const amostra = eJsonGKBANCO(new Uint8Array([0, 1, 2, 3, 4, 5, 6, 7])) === false
  return {
    slots: { in: SLOTS_DEPOSITO.in, out: SLOTS_DEPOSITO.out, disco: SLOTS_DEPOSITO.disco },
    opaco: opaco && slots && naoEstado && amostra,
  }
}

/**
 * B(id) = parte autorizada dos cortes χ_k, indexada por id.
 * F é fis:def:transf (mesmo chi que transformada.c). sha256 selecciona; não é F.
 */
function medeBandaCliente () {
  const t = medeTransformada()
  return {
    transformada_no_motor: t.transformada_no_motor,
    fonte: t.fonte,
    banda_canal: 'sha256',
    B_cliente: t.residuo === 0 ? 'medida' : 'nao localizada',
    parseval: t.parseval,
    volta: t.volta,
    residuo: t.residuo,
    corte: t.corte,
    homonimo: 'banda canal ≠ B_cliente; sha256 selecciona, nao e F; fis:thm:corte N/A',
  }
}

function medeParDocker (opts) {
  const docker = REALIZACOES.find((r) => r.id === 'docker')
  const spawnSync = opts && opts.spawnSync
  const sonda = spawnSync
    ? sondaDocker(spawnSync)
    : { disponivel: false, motivo: 'sem spawnSync' }
  let ciclo = { correu: false, estatuto: 'nao localizada', estado: null }
  if (sonda.disponivel && opts && opts.volumeHost) {
    ciclo = cicloDocker(estadoVazio(), {
      spawnSync,
      volumeHost: opts.volumeHost,
      sonda,
    })
  }
  return {
    docker_estatuto: docker ? docker.estatuto : 'nao localizada',
    sonda,
    ciclo,
    hash_igual_nao_implica: true,
  }
}

/**
 * Varredura determinística. Só extrai invariantes já medidos.
 * Nunca devolve estatuto realizado. Nunca inventa corpo.
 */
export function varrerLeiLocal (man, opts = {}) {
  const corpos = corposResiduoZero(man)
  const reals = realizacoesResiduoZero()
  const idsCorpo = corpos.map((c) => c.parte)
  const idsReal = reals.map((r) => r.id)

  const estado = medeEstado()
  const deposito = medeDeposito(man)
  const par = medeParDocker(opts)
  const banda = medeBandaCliente()

  const candidatos = []
  const alvos = []

  alvos.push({
    id: 'S_ESTADO',
    slots: estado.slots,
    contratos_residuo_0: idsReal.filter((id) => id === 'wasm' || id === 'remota'),
    nota: 'alvo de varredura; nao e lei realizada',
  })
  if (estado.iguais && idsReal.includes('wasm') && idsReal.includes('remota')) {
    candidatos.push(candidato(
      'L_S_ESTADO',
      'S_ESTADO',
      ['wasm', 'remota'],
      'hashCanonico sem t',
      'isoOperacional(cicloWasm, cicloRemoto); hash sem relogio; slots 9210/9211',
      'medida',
    ))
  }

  alvos.push({
    id: 'S_DEPOSITO',
    slots: deposito.slots,
    contratos_residuo_0: ['identidade.deposito', 'D_patria'],
    nota: 'alvo de varredura; payload opaco; != S_ESTADO',
  })
  if (deposito.opaco) {
    candidatos.push(candidato(
      'L_S_DEPOSITO',
      'S_DEPOSITO',
      ['identidade.deposito', 'D_patria'],
      'payload opaco',
      'SLOTS_DEPOSITO 9220/9221; deposito.bin; eJsonGKBANCO(bytes)=false; S_ESTADO perp S_DEPOSITO; sem transicao S0→S1 medida',
      'slots',
    ))
  }

  alvos.push({
    id: 'M_WASM_M_Docker',
    contratos_residuo_0: idsReal.filter((id) => id === 'wasm'),
    docker_estatuto: par.docker_estatuto,
    sonda: par.sonda,
    ciclo_correu: par.ciclo.correu,
    nota: 'alvo de varredura; M_Docker permanece nao localizada; hash(volume) nao promove',
  })
  if (par.ciclo.correu && par.ciclo.estado) {
    const S0 = estadoVazio()
    const S1 = aplicaInstrucao(S0, 'bash', 'echo ok', 'ok\n', T_FIXO)
    const hw = hashCanonico(cicloWasm(S1, memoriaLS()))
    const hd = hashCanonico(par.ciclo.estado)
    if (hw === hd && isoOperacional(cicloWasm(S1, memoriaLS()), par.ciclo.estado)) {
      candidatos.push(candidato(
        'L_S_WASM_Docker',
        'M_WASM_M_Docker',
        ['wasm', 'docker'],
        'hashCanonico sem t',
        'contentor mediou S1; M_Docker continua nao localizada (univ:def:maquina)',
        'medida',
      ))
    }
  }

  const promovidos = candidatos.filter((c) => c.estatuto === 'realizado')
  return {
    gramatica: 'univ:def:lei-local-canonica',
    caracterizacao: 'fis:thm:tecidos(1) fibra da face; Lei 7 ligar sem fundir',
    nao_e: [
      'X_{k+1}',
      'fis:thm:handshake',
      'analitico thm:tecidos como L_S operacional',
      'Ficha 11',
    ],
    corpos_residuo_0: idsCorpo,
    realizacoes_residuo_0: idsReal,
    alvos,
    candidatos,
    promovidos,
    docker: {
      estatuto: par.docker_estatuto,
      sonda: par.sonda,
      ciclo_correu: par.ciclo.correu,
    },
    ls_cliente: {
      estatuto: 'medida',
      transformada_no_motor: banda.transformada_no_motor,
      banda_canal: banda.banda_canal,
      B_cliente: banda.B_cliente,
      parseval: banda.parseval,
      residuo: banda.residuo,
      corte: banda.corte,
      homonimo: banda.homonimo,
      proibicao: 'nao promover corpo; M_Docker permanece nao localizada',
    },
    recusas: RECUSAS.slice(),
  }
}

export function semPromocoes (rel) {
  return Array.isArray(rel.promovidos) && rel.promovidos.length === 0 &&
    (rel.candidatos || []).every((c) => c.estatuto === 'nao localizada') &&
    rel.docker && rel.docker.estatuto === 'nao localizada' &&
    rel.ls_cliente && rel.ls_cliente.estatuto !== 'realizado' &&
    !(rel.candidatos || []).some((c) => c.id === 'L_S_cliente' && c.estatuto === 'realizado')
}
