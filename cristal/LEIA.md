# O cristal recuperado

A arqueologia do eval.txt (13/08): localizar → inventariar → proveniência →
extrair → gerar LaTeX → medir a volta. **A regra de ouro: não converter o
corpus antes de conseguir reconstruí-lo.**

## Onde estava

O cristal grande do projeto vivia no **broca-so**:
`broca-so/conversa/dados/conhecimento.graph.jsonl` — um jornal de **75.165
registos** onde cada conceito foi reescrito ao longo do tempo (ex.:
`floxina_investigacao`, 2908 versões). O jornal fica lá, intocado, com a
história inteira. Nota: `machine/tecido_prosa` (4,7G) **não** é o cristal —
é o tecido linguístico (4,2M frases Tatoeba + cache psi derivada).

## O que está aqui

- `cristal.jsonl` — a fonte: **4234 conceitos** (4286 recuperados, última
  versão de cada id, menos 52 fusões de curadoria), ordem canónica por id,
  JSON canónico (sort_keys, sem espaços). Cada registo carrega proveniência:
  `origem`, `meta.fonte`, `meta.dominio`, `epistemico`, `confianca`, e as
  `arestas` do grafo.
- Extraído por `tools/cristal_extrai.py`. Regra da casa aplicada
  (tools/corpo.sh: «IP privado — não publicar»): `user@IP` do servidor →
  `gex44`, 3 substituições, 0 restantes.
- `curadoria.tsv` — **o livro da curadoria** (resolvida a 13/08, ordem do
  dono): 52 fusões — 44 por **texto byte-idêntico** (o mesmo conteúdo por
  dois esquemas de endereço, id nu vs id de hub) + 8 **julgadas com leitura**
  (mesmo conceito em prosa diferente: os 4 mestres do xadrez com id duplo,
  processo/processos, reticulado/reticulados, Pisot, smart grid) — e 13
  pares **mantidos com motivo** (arte≠artes, análise⊋transformada de
  Fourier, homónimos). Aplicada por `tools/cristal_cura.py`; cada fusão
  guarda as duas partes intactas (`{"fusao":[x,y],...}`) e
  `--desfaz` devolve os 4286 **byte a byte** (verificado contra o git).
  Medidor: `tests/cristal_curadoria.js`.

## As projeções (LaTeX = projeção verificável, não fonte)

`tools/cristal_tex.py` gera `papers/cristal_*.tex` — 10 documentos por grupo
de domínios (63 domínios → 10 grupos), no formato da casa: uma `\section` por
conceito (título = fala, corpo = resposta — `tools/ingere.py` ingere direto).
Cada secção é precedida do registo original em comentário `%CRISTAL` — o
desenho do `/Type/FonteTeX`: a página é a leitura, a fonte viaja invisível.

| ficheiro | conceitos |
|---|---|
| cristal_ciencias.tex | 899 |
| cristal_matematica.tex | 679 |
| cristal_manual.tex | 516 |
| cristal_computacao.tex | 515 |
| cristal_engenharia.tex | 441 |
| cristal_papers.tex | 427 |
| cristal_fisica.tex | 332 |
| cristal_floxina.tex | 187 |
| cristal_xadrez.tex | 150 |
| cristal_diversos.tex | 88 |

(Os totais refletem a curadoria: manual −44 texto-idênticos, xadrez −4
mestres, matemática/computação/engenharia/diversos −1 cada.) Um registo de
fusão projeta-se como uma secção só: corpo da face mantida, a segunda face à
vista quando a prosa difere, e a nota de curadoria na proveniência.

Todos compilam com `pdflatex` (unicode exótico transliterado por tabela;
fallbacks contados à vista: 9 chars CJK de um exemplo japonês + controlos OCR).

## A volta (o medidor)

`tests/cristal_volta.js` — §V0–§V4: fonte íntegra; **reconstrução byte a byte
dos %CRISTAL contra cristal.jsonl com resíduo 0**; secções == registos por
dois caminhos; a perturbação de um byte acusa; portão do IP privado —
`#TOTAL 13 0` com o bloco de perturbações (quatro REOPEN à vista; reordenar
sobrevive por desenho). O estresse pela torre: `tools/lyapunov_measure.js`
(Teorema da Absorção, `papers/corpo_peano.tex`): e = R−k = 0 exato pelo
endereço; a divergência ~n² é da régua de posição, não do objeto.

## Refazer

```sh
python3 tools/cristal_extrai.py   # broca-so → cristal/cristal.jsonl (4286)
python3 tools/cristal_cura.py     # aplica a curadoria (52 fusões → 4234)
python3 tools/cristal_tex.py      # jsonl → papers/cristal_*.tex
node tests/cristal_volta.js       # a volta: resíduo 0
node tests/cristal_curadoria.js   # o livro, a conservação, a âncora
```
