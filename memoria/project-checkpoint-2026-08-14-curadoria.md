---
name: checkpoint-2026-08-14-curadoria
description: "A curadoria do cristal resolvida — 52 fusões reversíveis (44 medidas + 8 julgadas), 13 recusas com motivo; corpus 4286→4234; bateria 425:425; commit 36c1fa5 pushado."
metadata: 
  node_type: memory
  type: project
  originSessionId: b6c6c5cb-b5ec-45f0-ac00-480c20a1bb2d
  modified: 2026-08-14T04:15:24.429Z
---

**14/08 — A CURADORIA RESOLVIDA, 36c1fa5 (pushado).** Ordem do dono: «resolve a curadoria». Forma: **camada de transações declaradas**, lidas antes de decidir.

- **A leitura mudou a decisão**: dos 7 pares singular/plural, só 2 eram duplicados (processo, reticulado) — a regra cega teria destruído 5 conceitos distintos. E o estrato que valia mesmo não era o dos títulos: **44 pares de texto byte-idêntico** (o mesmo conteúdo ingerido por DOIS esquemas de endereço, id nu vs id de hub — `evidencia` = `birkeland_correntes_plasma_evidencia`; diferem só nas `arestas` reescritas e em `meta.fonte:"web"`). Descoberto medindo igualdade de corpo-sem-id: 0 grupos; de TEXTO (sem arestas/meta): 44.
- **52 fusões** = 44 texto-idêntico (mantém o endereço específico) + 8 julgadas (processo, reticulado, 4 mestres do xadrez com id duplo, Pisot, smart grid). **13 recusas com motivo** no livro (arte≠artes, análise⊋transformada de Fourier, homónimos como modelo_de_ameaca). A recusa é parte da resolução.
- Formato: o da operação medida — `{"fusao":[x,y],"id":mantido,"tipo":"conceito"}`, partes VERBATIM; o `%CRISTAL` re-serializa canónico e bate (dois caminhos assertados na ferramenta).
- **`tools/cristal_cura.py`** (aplica/`--desfaz`; regra A redescoberta a cada corrida, não lista morta) + **`cristal/curadoria.tsv`** (o livro: funde E mantem, com motivo) + **`tests/cristal_curadoria.js`** 7:0 (livro↔fonte; conservação nas 52; volta total a 4286 endereços e E=38.731.623.179 — a âncora pré-curadoria; gume por byte induzido; esgotamento: nenhum par texto-idêntico resta; controlo da recusa).
- Verificação de ouro: `--desfaz` == git HEAD **byte a byte**; reaplicar == primeira aplicação byte a byte.
- Armadilha real apanhada: `o_que_e` do par MANTIDO foi absorvido pelo gémeo idêntico `broca_os_hub_o_que_e` — o livro teve de citar o endereço sobrevivente; a ferramenta agora asserta que os mantidos SOBREVIVEM.
- E(fonte): 38.731.623.179 → **38.771.546.660** (= âncora + Σ contornos, derivado no medidor, não escrito à mão). Espectral 37.222 → **59.436** (âncora cruzada entre cristal_energia e equivalencia_universal). Grupos: manual 516 (−44), xadrez 150 (−4), matematica 679, computacao 515, engenharia 441, diversos 88.
- Atualizados: cristal_volta (4234), equivalencia (E derivada), fusao_conceitos (par demo = arte/artes mantido), observador_torre, cristal_tex.py (fusão desdobra: face mantida + segunda face à vista + nota), cristal.sh, LEIA.md, corpo_universal.tex (caixa de quarentena → curadoria resolvida), arquitetura.tex. **Bateria 425:425, zero falhas**; segredo.sh LIMPO.

Relacionado: [[checkpoint-2026-08-13-cristal]], [[feedback-a-referencia-escrita-a-mao]], [[feedback-dois-caminhos]].
