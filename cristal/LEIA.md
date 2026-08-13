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

- `cristal.jsonl` — a fonte: **última versão de cada um dos 4286 conceitos**,
  ordem canónica por id, JSON canónico (sort_keys, sem espaços). Cada registo
  carrega proveniência: `origem`, `meta.fonte`, `meta.dominio`, `epistemico`,
  `confianca`, e as `arestas` do grafo.
- Extraído por `tools/cristal_extrai.py`. Regra da casa aplicada
  (tools/corpo.sh: «IP privado — não publicar»): `user@IP` do servidor →
  `gex44`, 3 substituições, 0 restantes.

## As projeções (LaTeX = projeção verificável, não fonte)

`tools/cristal_tex.py` gera `papers/cristal_*.tex` — 10 documentos por grupo
de domínios (63 domínios → 10 grupos), no formato da casa: uma `\section` por
conceito (título = fala, corpo = resposta — `tools/ingere.py` ingere direto).
Cada secção é precedida do registo original em comentário `%CRISTAL` — o
desenho do `/Type/FonteTeX`: a página é a leitura, a fonte viaja invisível.

| ficheiro | conceitos |
|---|---|
| cristal_ciencias.tex | 899 |
| cristal_matematica.tex | 680 |
| cristal_manual.tex | 560 |
| cristal_computacao.tex | 516 |
| cristal_engenharia.tex | 442 |
| cristal_papers.tex | 427 |
| cristal_fisica.tex | 332 |
| cristal_floxina.tex | 187 |
| cristal_xadrez.tex | 154 |
| cristal_diversos.tex | 89 |

Todos compilam com `pdflatex` (unicode exótico transliterado por tabela;
fallbacks contados à vista: 9 chars CJK de um exemplo japonês + controlos OCR).

## A volta (o medidor)

`tests/cristal_volta.js` — §V0–§V4: fonte íntegra; **reconstrução byte a byte
dos %CRISTAL contra cristal.jsonl com resíduo 0**; secções == registos por
dois caminhos; mutação de um byte acusa; portão do IP privado. `#TOTAL 9 0`.

## Refazer

```sh
python3 tools/cristal_extrai.py   # broca-so → cristal/cristal.jsonl
python3 tools/cristal_tex.py      # jsonl → papers/cristal_*.tex
node tests/cristal_volta.js       # a volta: resíduo 0
```
