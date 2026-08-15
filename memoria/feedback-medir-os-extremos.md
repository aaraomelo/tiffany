---
name: feedback-medir-os-extremos
description: Medi `ker T* = (im T)°` varrendo 625 matrizes — os EXTREMOS da cadeia. Ele notou que se prova estruturalmente, e tinha razão: mede-se cada ELO.
metadata:
  type: feedback
---

Eu tinha medido `ker T* = (im T)°` varrendo 625 matrizes e comparando os dois conjuntos.
Verde, exaustivo, e **errado como método**. Ele escreveu:

> «Essa última já apareceu no teu corpus, mas agora dá para provar **estruturalmente, sem
> varredura**: φ ∈ ker T* ⟺ T*φ = 0 ⟺ φ(Tv) = 0 ∀v ⟺ φ|_{im T} = 0.»

A prova são **cinco definições em ⟺** — núcleo, igualdade pontual de funcionais, T* = φ∘T,
imagem, aniquilador. Nenhum cálculo. A varredura não estava errada: estava a medir a coisa
errada.

**Why:** varrer os extremos confirma que a conclusão é verdadeira, mas não mede a PROVA.
Se um elo estivesse errado e outro compensasse, a varredura não distinguia. É o mesmo
«não medir só a conclusão» que ele já tinha exigido para os teoremas — e eu tinha-o
aplicado às cadeias que ESCREVIA, mas não às que MEDIA.

**How to apply:** quando a prova é uma cadeia de equivalências, **medir cada seta**, e
deixar a coincidência dos extremos ser CONSEQUÊNCIA em vez de medida. Sinal de que estou a
fazer errado: a asserção compara dois conjuntos/valores calculados por caminhos
independentes, e a prova no texto tem cinco passos que ninguém verifica.

E o gatilho barato: **se a prova cabe em definições, a medida não precisa de varredura** —
e se eu estou a varrer, é porque não escrevi a prova. Ver [[feedback-dois-caminhos]]: ali a
comparação é o instrumento; aqui é o substituto de um.
