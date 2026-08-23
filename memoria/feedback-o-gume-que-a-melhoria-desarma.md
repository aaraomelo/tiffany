---
name: feedback-o-gume-que-a-melhoria-desarma
description: "Alarguei o envelope da célula e DOIS medidores verdes caíram — não por defeito, mas porque exigiam ver o limite TRAVAR e nada travava mais."
metadata:
  type: feedback
---

O envelope da célula subiu de −128..127 para −32768..32767. Dois medidores que estavam
verdes há meses passaram a falhar, e a asserção que caiu era esta:

```c
if(fora_env == 0){ printf("      → o envelope não travou nenhuma: o alcance"
                          " não foi exercido\n"); mal++; }
```

Ambos exigem ver o envelope **travar alguma coisa**. Com cinco letras o maior convergente
é 185: cabia mal em 127 e cabe folgado em 32767. Com m ∈ {1,2} nenhuma família de traços
chegava lá. **A lei não mudou — mudou onde ela deixa de caber**, e o regime onde o defeito
vive saiu de baixo do medidor.

**Why:** este é o DUAL de [[feedback-varrer-onde-nada-pode-falhar]]. Lá eu escolhia mal o
regime; aqui o regime estava certo e eu movi a fronteira. É o mesmo defeito visto do outro
lado: uma asserção sobre o ALCANCE é relativa a um limite, e mexer no limite desarma-a sem
tocar numa linha do medidor. E o sintoma engana: a bateria diz «falhou», o que se lê como
«partiste alguma coisa», quando o que aconteceu foi o contrário.

**How to apply:**

1. **Quando um limite sobe, procurar quem o media.** `grep` pelo valor antigo (127, 244) e
   por «alcance», «envelope», «travou», «não coube». As asserções que o citam são as que vão
   cair, e caem TODAS por esta razão, não por defeito.
2. **Curar é reencontrar o regime, não relaxar a asserção.** Tirar o `if(fora_env == 0)`
   seria matar a única coisa que garantia que o alcance foi exercido. A cura foi alongar o
   objecto até voltar a bater no tecto novo — e prever ONDE, em vez de tentar: com letras
   ≤ 3 o convergente só passa 32767 às onze letras, e isso conta-se antes de compilar.
3. **O alongamento pode trazer o objecto certo de graça.** Cinco letras a repetir-se até
   onze é a palavra PERIÓDICA — o quadrático. O medidor ficou a medir mais do que media.
4. **E os NÚMEROS DO VEREDICTO caem junto.** As strings do `ok()` diziam 77/77, 10/10,
   104/104 e «em DUAS delas o envelope trava». Passaram a 185/185, 20/20, 101/101 e «em
   UMA». Nenhuma asserção apanha isto — ver [[feedback-o-numero-no-veredicto]].

Medido em `tests/pgwire.c`, 23/08: «A PROPOSIÇÃO DOS TRAÇOS» e «A PALAVRA E A MATRIZ».
Ver [[project-checkpoint-2026-08-23-o-envelope]].
