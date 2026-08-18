---
name: feedback-o-ramo-que-nunca-corre
description: Mutação que sobrevive tem TRÊS causas — ramo inalcançável, guarda redundante, ou o gume mal feito por mim; e a terceira é a que eu leio como as outras duas.
metadata:
  type: feedback
---

Duas mutações sobreviveram à varredura, e as duas por causas **diferentes**. Só descobri
qual era qual porque fui **verificar em vez de adivinhar**:

1. **O ramo que nunca corre.** Na fração contínua, o quociente pelo CHÃO só se distingue
   do truncado com **numerador negativo** — em C o `/` trunca para zero, e para positivos
   as duas convenções coincidem. Eu varria 7140 frações, todas positivas: apagar a
   correção não mudava nada. Era um **gap meu**. Cura: entrar com negativos (14400), e a
   mutação morre.
2. **O guarda redundante.** No Teorema Chinês, a linha que recusa quando `gcd(m,n) ≠ 1` é
   o segundo fecho — o inverso modular já recusa sozinho. Apagá-la não muda o resultado, e
   isso **está certo**. Não é gap: é defesa em profundidade.

**Why:** «a mutação sobreviveu» não é um diagnóstico, é um sintoma com pelo menos duas
doenças. Tratar as duas como a mesma leva ou a apagar código correto, ou a dar por medido
o que não está.

**How to apply:** quando uma mutação não morde, escrever o **programa mínimo** que executa
a linha mutada e ver o que acontece — foram dez linhas e resolveu as duas. Depois:
- se a linha é alcançável e ninguém a testava → **o gap é meu**, alargar os dados até ao
  regime onde ela decide (é [[feedback-varrer-onde-nada-pode-falhar]] outra vez, agora ao
  nível da linha);
- se a linha é inalcançável porque outra a cobre → **dizer isso no comentário**, para o
  próximo leitor não a julgar carga útil.

E o gatilho barato: perguntar **que entrada faz esta linha decidir alguma coisa?** Se a
resposta não estiver nos dados que varro, a linha não está medida — mesmo com a bateria
verde. Ver [[feedback-assercoes-vazias]].


---

## 18/08/2026 — a TERCEIRA causa, e é a que eu leio como uma das outras duas

Duas e uma são diferentes: **o gume não mordeu porque o gume estava mal feito**. Aconteceu
duas vezes no mesmo dia, e as duas vezes eu já ia escrever «o código está certo».

**1. A guarda que eu dei por verificada sem apertar a régua.** Em `relogio_curva.c` pus uma
guarda de tecto e fui provar que ela servia — desliguei-a e corri. Deu `exit 0`, e eu li-o
como confirmação. Mas com a régua normal o grau fecha ao 8 e o tecto nunca é alcançado: o
meu teste não passava pelo sítio. Com a régua apertada, a mesma mutação dá **SIGSEGV**.

**2. O regex que só apanhava minúsculas.** Acabara de escrever uma conferência de
referências quebradas; para a testar, parti uma citação trocando-a por
`papers/corpo\_INEXISTENTE.tex`. A conferência calou-se. Por um momento pareceu que o
detector não servia — e o que não servia era o teste: o regex procura `[a-z0-9]`, e eu
tinha escrito em MAIÚSCULAS. Com `corpo\_fantasma` acusa na hora.

### A ordem certa quando um gume não morde

Antes de concluir «ramo inalcançável» ou «guarda redundante», perguntar:

1. **O meu gume passa pelo sítio?** — o caso de teste alcança a linha mutada? (a régua
   apertada, o dado que força o ramo)
2. **O meu gume tem a FORMA que o código procura?** — maiúsculas/minúsculas, escapes do
   LaTeX (`\_`), o separador certo, o prefixo certo
3. **A mutação foi mesmo aplicada?** — o `diff` acusa linhas mudadas? (já me aconteceu o
   `sed` não casar e eu ler o verde como «não morde»)

Só depois de as três passarem é que a sobrevivência diz alguma coisa **sobre o código**.

E o sintoma comum às duas: **eu escrevi o teste sabendo a resposta que queria**, e por isso
não olhei para ele com a mesma desconfiança com que olho para o código. Vale o mesmo que
[[feedback-assercoes-vazias]] diz da nona forma — o momento de maior risco é o da correcção.


---

## E o mesmo vale para o DETECTOR: o meu, hoje, acusou 20 e 15 eram ruído

Escrevi uma ferramenta para conferir caminhos citados em workflows e scripts. Primeira
corrida: **20 caminhos quebrados**. Fui investigar um a um — e quinze eram defeito do
regex, não do repositório:

```python
r'\.(?:tex|c|js|json|…)'        # `\.c` casa com o INÍCIO de `.claim`
                                #  `.json` casa com o início de `.jsonl`
```

Doze «quebrados» eram `conecthus/claims/pareto.claim` lidos como `…/pareto.c`. Outro era
`docs_tradutor.json → tex_tradutor.js` — dois ficheiros numa frase, colados num caminho
que nunca existiu. Com `(?![A-Za-z])` a fechar a extensão, ficaram os **cinco**
verdadeiros.

### Por que é que isto importa mais do que parece

Um detector que acusa três vezes mais ruído que sinal **treina quem o lê a ignorá-lo**. Se
eu tivesse posto aquela versão na bateria, a linha «20 caminhos quebrados» passaria a ser
paisagem, e os cinco reais morriam lá dentro. É o mesmo que uma asserção que nunca falha,
ao contrário: uma que falha sempre.

**Antes de ligar um detector à bateria, investigar TODOS os achados da primeira corrida.**
Se a maioria for ruído, o detector não está pronto — e o número que ele imprime é pior que
nenhum, porque parece informação.

Vale para as três ferramentas de hoje: `tools/tautologia.py` (filtrar declarações
múltiplas e parâmetros de saída), `tools/refcruz.py` (comentários fora), `tools/caminhos.py`
(a extensão fechada). Nas três, a primeira versão acusava coisas que não eram.
