---
name: feedback-estrutura-lida-como-ruido
description: "Um resíduo que não fecha pode ser MEIA ÓRBITA, não erro de medida — perguntar o período ANTES de propor medir dispersão"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-05T21:05:22.410Z
---

O `tresp.c` dava `ν∘ν` com resíduo `0,402` contra um controlo de `0,421` — quase iguais. Diagnostiquei: *«a asserção exige zero de um operador estocástico; é preciso medir a dispersão primeiro»*. **Estava errado, e o Aarão desfez o diagnóstico com uma pergunta: «talvez aí caiba a bidualidade — 4 operadores duais, já considerou?»**

Não considerara. E a leitura certa não precisa de ruído nenhum: se o objecto tem **dois lados**, aplicar a mesma involução duas vezes leva ao **antípoda**, não a casa. O resíduo não era erro de medida — era **meia órbita**. E os números diziam-no: ida e controlo *quase iguais* significa que o ponto final está tão longe da partida quanto o ponto do meio, que é exactamente o que uma órbita de quatro faz.

**Why:** medir dispersão é caro, e teria confirmado ruído que não existia — e o pior: teria «validado» um limiar novo em cima de um percurso errado, que é o defeito de procurar o número que faz a asserção passar, uma camada acima.

**How to apply:** perante uma involução que não fecha, a **primeira** pergunta é *qual é o período do operador* — não *qual é o ruído da medida*. Provar a ordem (`ordem == 2`, como o `gauss.c` faz) em vez de a supor. Só depois de a ordem estar provada é que a hipótese de ruído faz sentido.

Sinal de reconhecimento: **o resíduo da «volta» é da mesma ordem do controlo**. Se voltar mesmo, tem de ser *muito* menor. Igual ao controlo é meia órbita.

Varrido em `tools/involucoes.sh`: 50 medidores afirmam involução, 27 pedem leitura, 15 lidos — os limpos **provam** a ordem ou reutilizam uma involução já provada; o `tresp` é o único que a supõe.

Ver [[feedback-medir-a-estabilidade-antes]] (que é a regra oposta e continua válida — o erro foi aplicá-la antes de verificar a estrutura), [[feedback-a-base-ja-existe]], [[project-a-lei-em-dois-niveis]].
