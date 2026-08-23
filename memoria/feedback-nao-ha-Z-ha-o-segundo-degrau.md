---
name: feedback-nao-ha-Z-ha-o-segundo-degrau
description: "Não há ℤ na aranha — há X_2, o segundo degrau, CONSTRUÍDO. «Escrever Z seria usar o que se vai construir.»"
metadata:
  type: feedback
---

Escrevi «a solução vive em **ℤ[ω]**» e o Aarão parou-me: *«me diz onde tem Z na teoria da
aranha»*. Não tem. O preâmbulo do `aranha.tex` define-o e diz porquê:

```latex
%% O SEGUNDO DEGRAU, e não um conjunto importado: este documento CONSTRÓI-O
%% (§escada, X_2 = X_1^2/~). Escrever Z seria usar o que se vai construir.
\newcommand{\Zc}{\ensuremath{X_2}}
```

O paper usa `\mathbb{Z}` **três vezes em 4374 linhas**, e uma delas é para dizer que *«ℤ, ℚ e
ℝ são degraus posteriores da §escada, e nenhum deles é preciso para isto»*.

**Why:** importar o conjunto dispensa a escada que o produz — e a escada é o conteúdo. É a
mesma falha de «a base já existe» e «inventei em vez de procurar», mas na porta de entrada:
escrever ℤ é começar depois do princípio.

**How to apply:**
1. **Os coeficientes são de `X_2`**, o quociente por `(a,b)~(c,d) ⟺ a+d=b+c` que fecha a face
   do OPOSTO. Escreve-se `X_2 + X_2·ω`, não `ℤ[ω]`.
2. **Nenhum conjunto numérico entra por nome.** ℕ, ℤ, ℚ, ℝ são degraus — `X_1`..`X_4` — e
   cada um chega quando a escada o constrói.
3. E o mesmo vale para os NOMES de fora: chamei «convolução de Cauchy» à convolução do
   `def:conv`, que é a mesma que o `thm:zeta-mu` corre como gato e esquilo. O nome clássico
   entra como cláusula, se entrar — ver [[feedback-o-sujeito-da-frase]].
4. Quando aparecerem muitas ocorrências antigas, **contá-las e dizer**, em vez de um
   `replace` em massa — ver [[feedback-o-replace-sem-limite]]. No `catalogo.tex` são 124.
