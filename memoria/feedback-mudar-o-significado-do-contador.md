---
name: feedback-mudar-o-significado-do-contador
description: "Mudar o SIGNIFICADO de um contador partilhado quebra quem o lia, e a quebra aparece longe — em asserções que nem falam dele."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: cee0686a-c852-4d69-8fca-ca206e1fba24
  modified: 2026-08-20T22:45:13.129Z
---

Em 20/08 a decisão «saturo→promove» trocou o `Qz` de int16 para int64. O código
mudou pouco. O que mudou muito foi o SIGNIFICADO de `qz_saturou`: antes «saiu de
E₁₆» era **truncou**, depois passou a ser **subiu de andar com o valor intacto**.

**SEIS medidores caíram, e nenhum deles falava de saturação.** Falavam de Cauchy,
de retração FC e de séries-p. `cy_teto_honesto` lia `qz_saturou` para saber até
onde a sucessão ainda É a sucessão — e passou a cortar termos bons (a promoção)
enquanto deixava passar os maus (o encaixe congelado). `c2_soma_segura` tirou o
teste `x.saturo`, não pôs nada no lugar, e voltou a deixar passar a soma
descartada que o texto da própria asserção denuncia.

**Why:** um contador partilhado é uma INTERFACE. Quem o lê não pergunta o número:
pergunta o predicado que ele codifica. Trocar o predicado sem renomear é trocar
a interface em silêncio — o compilador não vê, as asserções locais não veem, e
quem cai é quem estava a três ficheiros de distância.

**How to apply:** antes de mudar o que um contador conta, `grep` por ele em todo o
repo e ler o PREDICADO de cada leitor («ainda é o valor?», «coube?», «promoveu?»).
Se dois leitores querem predicados diferentes, são DOIS contadores, não um —
`qz_saturou` (promoveu, exacto) e `qz_perdeu` (descartou, não é o valor). E a
regra da casa aplica-se ao contador como a tudo: [[feedback-definicao-e-medida]] —
o que PODE acontecer e o que ACONTECEU não cabem na mesma gaveta.

O sintoma que a denuncia: uma função de guarda que passou a devolver sempre 1, ou
um tecto honesto que subiu sem que nada tivesse melhorado. Ver
[[feedback-saturacao-nao-e-resultado]] e [[feedback-medir-so-metade-do-par]].
