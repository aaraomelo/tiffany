---
name: project-g-constante-nao-certifica-a-leitura
description: "A completude (G constante) é condição sobre a FIBRA e não certifica que a leitura seja a do corpo — medido no quebra.c."
metadata:
  type: project
---

Ao correr o pipe sobre as entradas do catálogo, o `quebra.c` deu **G ≡ 5, falta 0** — quociente
perfeito — e mesmo assim a leitura é a **errada**: junta **50 pares** de objectos diferentes e
parte **30 pares** de objectos iguais. O `quebra.c` existe precisamente porque o Aarão parou uma
correcção minha com *«você está a consertar — falei pra QUEBRAR»*, e o que ele quebrou foi eu ter
ordenado o **parâmetro** em vez do **corpo**.

**Why:** eu estava a tratar «G constante» como certificado de que o corpo estava bem lido. Não é.
São duas perguntas independentes:

| pergunta | quem responde |
|---|---|
| a fibra é uniforme? | `es_fibra` → G constante |
| a leitura é a do OBJECTO? | o critério: bem definida **e** separadora |

**How to apply:**
1. Depois de `es_fibra` dar `constante`, correr **sempre** `tv_criterio` com a igualdade **do
   corpo** — não a dos parâmetros. Uma leitura pode ser quociente exacto de uma partição que não
   é a do objecto.
2. O sintoma é o mesmo de [[feedback-contar-objectos-pelos-parametros]]: a resposta pressuposta
   na pergunta. Aqui aparece com cara de resultado bom (falta 0), que é pior.
3. O gume que apanha isto: contar os pares que a leitura **junta** sendo diferentes e os que
   **parte** sendo iguais. Se ambos forem zero, a leitura é a do corpo; se G for constante e
   estes não forem zero, é quociente da partição errada.

Medido em `tests/pgwire.c` §W173. Ver [[project-a-navegacao-herda-a-regua]] e
[[feedback-a-definicao-do-extractor]].
