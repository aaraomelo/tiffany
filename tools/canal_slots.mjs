/* canal_slots.mjs — slots S_CANAL (igual banco/sql.c e canal_browser.js). */
export const ISA_TECTO = 1 << 16
export const S_CANAL = ISA_TECTO + (600 << 14)
export const S_BASH_IN = S_CANAL + 9100
export const S_BASH_OUT = S_CANAL + 9101
export const S_CHUNK = S_CANAL + 9102
export const S_PWSH_IN = S_CANAL + 9110
export const S_PWSH_OUT = S_CANAL + 9111
export const S_NODE_IN = S_CANAL + 9120
export const S_NODE_OUT = S_CANAL + 9121
export const S_FRONT_REQ = S_CANAL + 9200
export const S_FRONT_RSP = S_CANAL + 9201
export const S_ESTADO_REQ = S_CANAL + 9210
export const S_ESTADO_RSP = S_CANAL + 9211
/** Blob opaco de utilizador (D_patria). ≠ S_ESTADO (JSON GKBANCO operacional). */
export const S_DEPOSITO_REQ = S_CANAL + 9220
export const S_DEPOSITO_RSP = S_CANAL + 9221
/** Contrato N-partes no fio. ≠ S_ESTADO. Chunk próprio para não misturar o shell. */
export const S_COORD = S_CANAL + 9230
export const S_COORD_CHUNK = S_CANAL + 9232
/** Worker i → coord: par próprio, para as peças não se entrelaçarem no bus. */
export function slotPeca (i) { return S_CANAL + 9240 + ((i | 0) * 2) }
export function slotPecaChunk (i) { return S_CANAL + 9241 + ((i | 0) * 2) }
