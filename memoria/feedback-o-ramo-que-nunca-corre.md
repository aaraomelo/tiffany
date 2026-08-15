---
name: feedback-o-ramo-que-nunca-corre
description: A mutação sobreviveu porque o ramo mutado nunca chegava a correr com os dados que eu varria — e distinguir isso de «guarda redundante» é o trabalho.
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
