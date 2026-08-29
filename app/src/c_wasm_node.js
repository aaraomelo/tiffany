// c_wasm_node.js — compat: reexporta c_wasm_shell (cadeia node).
export {
  cadeiaShell as cadeiaNode,
  capasCadeia,
  paridadeWasmMetal,
  paridadeCanalShell as paridadeCanalNode,
  traduzCadeiaNode,
  moveShellArena as moveNodeArena,
  traduzCadeia,
} from './c_wasm_shell.js'
