---
name: feedback-medir-a-estabilidade-antes
description: "Três vezes num dia tirei lei de amostra pequena sem verificar se o número era estável — e uma delas foi publicada errada"
metadata:
  node_type: memory
  type: feedback
  modified: 2026-08-02T18:50:00.000Z
---

# Medir a estabilidade ANTES de escrever a lei

02/08/2026, a crescer o tecido da assistente. **Três medições do mesmo tecido enganaram-me no mesmo
dia, e as três foram o mesmo erro de fundo:** tirar conclusão de uma amostra pequena sem verificar
se o número era estável.

1. **O prefixo** — o cosseno subiu ao crescer e eu ia escrever uma segunda lei por cima da primeira.
   A causa era um prefixo constante que eu próprio pusera. Ver
   [[feedback-a-chave-faz-parte-da-medida]].
2. **O ruído ortogonal** — a camada do `pdftotext` tinha 392 falas duplicadas e chaves de lixo, e
   ainda assim melhor cosseno: *ruído é ortogonal*, e cosseno baixo não é riqueza.
3. **A amostragem** — o pior, porque **foi publicado**. Escrevi que o enredo fizera o `tan φ`
   subir (1,7121 → 1,7289) e construí a lei "material de outro domínio acrescenta capacidade". Com
   amostragem correta, o `tan φ` **desce monotonamente nas três colheitas** (1,7564 → 1,7051 →
   1,6885 → 1,6407). A lei era falsa.

**O defeito técnico concreto, que vale conhecer:** `V[::len(V)//n]` parece amostragem espalhada, mas
com `len(V) = 400` e `n = 300` o passo é **1** — devolve os **primeiros 300 consecutivos**, todos da
mesma zona do ficheiro. Só funciona quando `len >> n`. O certo é
`V[np.linspace(0, len(V)-1, n).astype(int)]`.

**Why:** um escalar medido em 24 pontos varia mais do que o efeito que eu quero detetar. No mesmo
tecido, medindo com n = 24/40/100/300/400, o `tan φ` variou até **0,27** — e as diferenças sobre as
quais eu estava a legislar eram de 0,02 a 0,05. **O ruído da medida era cinco vezes maior que o
sinal**, e eu não tinha olhado.

**How to apply:** antes de comparar duas medições e escrever o que a diferença significa, medir **o
mesmo objeto com dois ou três tamanhos de amostra**. Se o número se mexe mais do que a diferença que
quero explicar, não há nada a explicar ainda — aumentar a amostra primeiro. E desconfiar de qualquer
`[::passo]` onde o passo pode dar 1. Liga com [[feedback-dois-caminhos]] (dois caminhos que têm de
concordar — aqui os "dois caminhos" são dois tamanhos de amostra) e com
[[feedback-verdadeiro-e-parcial]].
