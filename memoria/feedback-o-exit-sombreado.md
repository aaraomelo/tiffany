---
name: feedback-o-exit-sombreado
description: 17 medidores verdes com unidades vermelhas — um contador local sombreava o do header; a cura é a REDE no runner, não confiar na canalização de cada um
metadata:
  type: feedback
---

O cards.c saiu 0 com duas `#UNIT falha`: um `long falhas = 0` local no main sombreava o `static int falhas` de lib/unidade.h — o `ok()` somava no do header, o `return` devolvia o local. Dezasseis outros tinham o mesmo shadow; três devolviam `return 0` fixo. Nenhuma asserção o apanharia: o defeito era a própria canalização do veredito.

**Why:** um medidor com o exit desligado das unidades é um [[feedback-assercoes-vazias]] de corpo inteiro — passa sem poder falhar. E o grep que fiz primeiro («quem devolve 0 fixo?») deu 10 falsos positivos e falhou os verdadeiros: texto não é comportamento.

**17/08 — voltei a ESCREVER o defeito, num medidor NOVO.** O `cuspide.c` nasceu com `long falhas = 0` no main e `return falhas ? 1 : 0`. Ele imprimiu DUAS `NÃO ✗` no ecrã, eu li o output, e o `exit=0` na linha seguinte não me disse nada. A cura do runner não me protege enquanto o ficheiro não é citado — e um medidor novo passa horas fora da bateria. **Ao escrever um medidor: não declarar `falhas`. Ela vem do header.**

**How to apply:** a cura de classe fica no RUNNER, não em cada ficheiro: a bateria compara exit com `#UNIT falha` nos 3 ramos e declara «VERDE FALSO» — [[feedback-dois-caminhos]] aplicado ao próprio veredito. E quando a rede morde, REATESTA na hora (o reuso ressuscitava o verde — [[feedback-o-medidor-que-nunca-mediu]]). Sinal de alerta barato: o total diz «N verdes» e a linha das unidades diz «2 falharam» — ler AS DUAS linhas, sempre.
