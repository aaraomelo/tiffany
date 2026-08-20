// ── O CORPO NO FRONT: os ficheiros de que o tradutor precisa, servidos de onde estão ──
//
// O Aarão: «põe os arquivos que vc precisa no front, pode ser?»
//
// A lista não é minha: está em `corpo.json` e sai de `tools/corpo.sh`, que intercepta o
// `fopen` do tradutor e escreve o que ele foi mesmo buscar para compor os documentos do front.
// Uma lista à mão envelhece calada — muda-se um `\fontsize` no estilo, entra um corpo novo
// que ninguém pôs no manifesto, e o pedido cai com um 404 que não explica nada.
//
// NADA É COPIADO PARA O DIST À PARTIR DO REPO. O Vite serve /corpo/* do BANCO
// (`.torre/reino_corpo`, populado por `IMPORT CORPO` no sql.c). O manifesto
// continua a ser o PORTÃO: só sai o que tools/corpo.sh mediu.
//
// ── E O MANIFESTO É O PORTÃO ──────────────────────────────────────────────────────────
//
// O repositório é público, mas nem tudo nele é para servir: o `curriculo/` tem CPF e conta
// bancária, e o `broca-so/cristalchain` diz «IP privado — não publicar». Uma rota estática
// sobre a raiz servia isso tudo com um 200 tranquilo. Por isso a regra é de LISTA e não de
// padrão: sai o que o tradutor provou precisar, e mais nada. Um padrão do género
// «recusa /curriculo/» é uma lista negra, e uma lista negra esquece-se do próximo segredo.
import manifesto from './corpo.json'

export const FICHEIROS = manifesto.ficheiros
const PERMITIDOS = new Set(FICHEIROS)

// os tipos que estes ficheiros são. Não há aqui adivinhação: são três extensões e mais nada.
const TIPOS = {
  '.tex':  'text/plain; charset=utf-8',
  '.txt':  'text/plain; charset=utf-8',
  '.otf':  'font/otf',
  '.json': 'application/json; charset=utf-8',
}

export function tipoDe (caminho) {
  const p = caminho.lastIndexOf('.')
  return (p < 0 ? null : TIPOS[caminho.slice(p)]) || 'application/octet-stream'
}

// Devolve o caminho RELATIVO À RAIZ que o pedido nomeia, ou null se não é para servir.
// Decodifica-se ANTES de decidir: sem isso, `%2e%2e%2f` passava o portão e só depois virava
// `../` na leitura — o portão tem de ver o mesmo caminho que o disco vai ver.
export function resolveNoCorpo (url) {
  if (typeof url !== 'string') return null
  const m = /^\/corpo\/(.+?)(?:\?.*)?$/.exec(url)
  if (!m) return null
  let caminho
  try { caminho = decodeURIComponent(m[1]) } catch { return null }
  if (caminho.includes('\0')) return null
  return PERMITIDOS.has(caminho) ? caminho : null
}
