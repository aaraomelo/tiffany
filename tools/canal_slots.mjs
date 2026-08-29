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
