# A bancada de integração — a crónica do doador

Saneamento de 14/08 (decisão do coordenador, protocolo do gerente, selo do
diretor): **o conceito de doador saiu do caminho de prova**. «Se para
verificar a lei precisamos pedir a resposta a outro agente, ainda existe uma
dependência arquitetural que não deveria estar no núcleo.»

O crivo aplicado a cada um dos 10 (a caixa do gerente):

    doador → derivável?  → reescrever interno
           → fixture?    → dado versionado com proveniência
           → integração? → esta bancada (fora da bateria de leis)
           → nada mede?  → remover

**Veredito: os 10 são INTEGRAÇÃO** — medem dados reais de um doador vivo
(qwen2.5:1.5b + nomic-embed-text via Ollama), colhidos por scripts que na
sua maioria já saíram do repo na purga auto-contida de 05/08. Nenhum foi
removido: cada um regista um ACHADO histórico do experimento da transfusão,
e o texto de cada fonte é a crónica. Os **gémeos internos** das operações
já vivem na bateria: `tests/transfusao.c` (o procedimento com dados
próprios), `banco`/plugue (a volta byte a byte), a involução e o bump em
`banco/fala.c -teste`.

| medidor | o achado histórico | precisa |
|---|---|---|
| antissim.c | o laço antissimétrico fechou em órbita de PERÍODO 2 | ollama + antissimetrica.sh (saiu) |
| campomedio.c | estabilidade quase-áurea do campo médio (critério de outra natureza que ν∘ν=0) | ollama + interroga.sh (saiu) |
| cifrando.c | a cifra do ponto É o endereço (Lagrange: periódica ⟺ quadrática) | embeddings colhidos |
| dualcifra.c | ele soma, nós multiplicamos — completar a dualidade da cifra | embeddings colhidos |
| entrega.c | o gabarito verificável: 3 de 12 erradas por sinal textual decidível | veste_doador.sh (saiu) |
| folhas.c | a pergunta certa é ONDE CABE, não passa/não-passa | embeddings colhidos |
| liquida_doador.c | o metal do doador não se declara; PROCURA-SE (varre m) | interroga.sh (saiu) |
| protocolo.c | ν∘ν=0 exige unicidade que a linguagem não tem — pulou 8 de 8 | protocolo.sh (saiu) |
| recupera.c | autocompletar = o banco devolver o que lá foi posto | banco/grava_saber.sh + ollama |
| transfusao_real.c | o doador não tem por que fechar; mede-se QUANTO fecha | colhe_transfusao.sh (saiu) |

**A Regra do Oráculo** (diretor): dependência externa indispensável habita
esta bancada, nunca a bateria de leis. Para reproduzir: doador Ollama
acordado + recriar os scripts de colheita; se um dia se colher de novo,
versionar os vetores como FIXTURE com hash e proveniência — aí partes desta
crónica podem voltar à bateria como dados declarados.

As linhas destes medidores em `tools/atestados.txt` ficam (a tabela nunca
se trunca): registam o resultado da última medição com o doador vivo.

## Anexo (14/08, mesma sessão): integração de servidor

- **compoe_ao_clicar.js** — precisa do vite vivo em :8099 (`cd app && npx
  vite preview --port 8099`); mede o clique→PDF de ponta a ponta. Ambiente,
  não matemática: mora aqui pela mesma Regra do Oráculo.
