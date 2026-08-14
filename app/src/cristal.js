// cristal.js — chip X (xtal): a proveniência do cristal na tela.
// O cristal (cristal/cristal.jsonl, 4234 conceitos após a curadoria) chega à assistente pelo
// BANCO (tools/cristal.sh ingere as projeções); o chip não pede ficheiro novo
// ao site — lê a resposta e mostra o recibo: origem · fonte · domínio ·
// confiança · história. A memória não é só armazenamento; tem auditoria.
//
// E o portão do coordenador: «a energia carrega assinatura; o primeiro bit
// decodificado incorretamente já rejeita a iteração» — a validação pára no
// PRIMEIRO byte errado (rejeitaPrimeiroErro), e a assinatura da resposta é
// energia+fase (o módulo e a posição — a transposição não engana a fase).

/** A fala pede o cristal / a proveniência? */
export function pedeCristal (fala) {
  const t = String(fala || '').toLowerCase()
  return /(mostra o cristal|de onde (veio|vem)|proveni[eê]ncia|hist[oó]ria d[eo]|quantos conceitos)/.test(t)
}

/**
 * Extrai a proveniência que as projeções carregam e o ingere levou ao banco:
 * «proveniência: <origem…> confiança <c> domínio <d> [história <n> versões…]».
 * Decodificação com portão: campo ilegível → null (rejeita a iteração).
 */
export function extraiProveniencia (resposta) {
  const t = String(resposta || '')
  const m = /proveni[eê]ncia:\s*(.+?)\s+confiança\s+([\d.]+)\s+domínio\s+(\S+)(?:\s+história\s+(\d+)\s+versões)?/.exec(t)
  if (!m) return null
  const conf = Number(m[2])
  if (!Number.isFinite(conf)) return null      /* primeiro campo mal decodificado: rejeita */
  return {
    origem: m[1].trim(),
    confianca: conf,
    dominio: m[3],
    versoes: m[4] ? Number(m[4]) : 1,
  }
}

/** Assinatura da representação: energia (Σb²) e fase (Σ i·b), mod 65537 —
 * o anel da Lei 8. A fase apanha a transposição que a energia não vê. */
export function assinatura (texto) {
  const P = 65537
  let E = 0, fase = 0, n = 0
  const s = String(texto || '')
  for (let i = 0; i < s.length; i++) {
    const b = s.charCodeAt(i) & 0xFFFF
    E = (E + b * b) % P
    fase = (fase + (i + 1) * b) % P
    n++
  }
  return { E, fase, n }
}

/** O portão: devolve a posição do PRIMEIRO byte diferente (−1 se iguais).
 * Rejeita à primeira — não percorre o resto. */
export function rejeitaPrimeiroErro (fonte, candidato) {
  const a = String(fonte || ''), b = String(candidato || '')
  const n = Math.min(a.length, b.length)
  for (let i = 0; i < n; i++) {
    if (a.charCodeAt(i) !== b.charCodeAt(i)) return { ok: false, pos: i }
  }
  if (a.length !== b.length) return { ok: false, pos: n }
  return { ok: true, pos: -1 }
}

/**
 * O passo do chip: dada a fala e a resposta emitida, o recibo para a tela.
 * Sem proveniência na resposta não inventa: diz que a resposta não veio do
 * cristal (ok=false só quando a fala PEDIU proveniência e ela não há).
 */
export function passoCristal (fala, resposta) {
  const prov = extraiProveniencia(resposta)
  const pediu = pedeCristal(fala)
  if (!prov && !pediu) return null
  const ass = assinatura(resposta)
  if (!prov) {
    return {
      tipo: 'cristal',
      ok: false,
      prov: null,
      ass,
      motivo: 'a resposta não carrega proveniência do cristal',
    }
  }
  return {
    tipo: 'cristal',
    ok: true,
    prov,
    ass,
    motivo: prov.origem +
      ' · domínio ' + prov.dominio +
      ' · confiança ' + prov.confianca +
      (prov.versoes > 1 ? ' · história ' + prov.versoes + ' versões' : ''),
  }
}
