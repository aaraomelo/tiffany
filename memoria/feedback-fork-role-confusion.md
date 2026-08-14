---
name: feedback-fork-role-confusion
description: "Forks herdam o meu papel de orquestrador e role-confundem-se; para editar ficheiros em paralelo usar agentes frescos, não forks."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c6388688-fee0-4332-86fe-642d1b68c27c
  modified: 2026-08-09T20:08:21.417Z
---

Lancei QUATRO forks (subagent_type: "fork") para editar quatro papers em paralelo, um por ficheiro. Como o fork herda o meu contexto INTEIRO — incluindo o meu papel de «orquestrador à espera dos outros forks» —, cada um julgou-se a mim: um reportou «há 4 forks a correr, paro todos», outro editou o ficheiro ERRADO (o «teoria» fez arquitetura e foi para dualsort), e um tentou `TaskStop` nos irmãos e foi morto a meio, deixando ficheiros em estado parcial. Cascata de role-confusion.

**Why:** o fork é para HERDAR contexto e continuar o MEU raciocínio como se fosse eu — por isso adota o meu papel. Quando esse papel é «coordenar N tarefas», o fork coordena em vez de executar a sua. Um agente `general-purpose` fresco (contexto próprio, sem o meu papel) fez a coisa certa: foi direto à fonte medir o número.

**How to apply:** para editar ficheiros independentes em paralelo (um agente por ficheiro), usar `general-purpose` com prompt AUTO-CONTIDO (a reconciliação, o alvo, o critério) — nunca `fork`. Reservar o fork para quando quero mesmo um clone meu a continuar o MESMO fio, sozinho. E [[feedback-destruir-antes-do-inventario]]: um agente confuso pode escrever por cima; depois de parar tudo, `git diff --stat` e rever CADA edição de agente (podem ter editado o ficheiro certo com bom conteúdo, ou o errado). Liga-se a [[feedback-revisores-externos]] (paralelo compensa, mas é preciso governá-lo) e [[feedback-o-write-que-diz-updated]].
