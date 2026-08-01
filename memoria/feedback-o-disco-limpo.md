---
name: feedback-o-disco-limpo
description: O runner limpo não é mais rigoroso — ele só não tem o meu passado. Defeitos que só aparecem sem o estado que corridas anteriores deixaram
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T17:59:37.485Z
---

# O disco limpo não é mais rigoroso — ele só não tem o meu passado

01/08/2026. O deploy do chess falhou **quatro vezes**, e **nenhuma** era o pdflatex, que era o meu
diagnóstico. Três das quatro eram a mesma coisa por baixo: **aqui em casa passava porque o disco já
tinha o resultado de eu ter corrido aquilo à mão, uma vez, há muito.**

A pior, e a que ensina:

> `manual_pdf.py` e `integra_docs.py` **conferem cada um OS TRÊS PDFs no fim, e cada um só gera
> dois.** Num disco vazio o primeiro a correr falha sempre, seja qual for a ordem — ele confere um
> ficheiro que o outro ainda não escreveu. Inverter a ordem só trocou **qual** deles falhava.

Eu nunca vira isso em dezenas de corridas locais, porque os três PDFs estavam no disco desde a
primeira vez. **O ciclo tinha um ponto morto e o meu disco escondia-o.**

## O padrão, que é maior que o CI

É o mesmo defeito do dia inteiro, por três portas:

- o **`léxico`** só sumia no documento inteiro — no fragmento isolado saía bem
- o **`$` do verbatim** fazia o dano nascer na linha 1532 e aparecer na 2071
- os **três PDFs** só falhavam sem o estado que eu tinha acumulado

**Em todos: o defeito não vive onde eu o procuro, e o meu ambiente tem estado que o mascara.**

## Como aplicar

1. **Quando algo passa localmente e falha no CI, a primeira hipótese não é "o CI está mal
   configurado" — é "o meu disco tem coisa que o dele não tem."** Perguntar: que ficheiro existe
   aqui que lá não existe? Que passo eu corri uma vez à mão e nunca mais?
2. **Reproduzir num diretório vazio antes de culpar o ambiente remoto.** Um `git clone` para
   `/tmp` e correr lá teria achado as três em minutos.
3. **Um script que GERA e depois CONFERE o conjunto todo tem um ponto morto por construção** se
   outro script gera parte do conjunto. Separar explicitamente a passada de geração da de
   verificação — e dizer nos comentários qual é qual.
4. **O `|| true` é quase sempre desonesto, mas há um caso em que não é:** a passada declaradamente
   de geração, cujo veredito não pode valer porque falta um ficheiro *por construção*. Fora disso,
   ele esconde — e nesta mesma sessão eu tinha posto um `2>/dev/null || true` num `pip install`
   três linhas abaixo do meu próprio comentário a dizer *"um passo que pode falhar tem de deixar
   ver porquê"*.

Ver [[project-publicacao-patria]], [[feedback-dois-caminhos]], [[project-compilador-tex]].
