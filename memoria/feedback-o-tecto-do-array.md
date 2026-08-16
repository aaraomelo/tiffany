---
name: feedback-o-tecto-do-array
description: "Subi o array até 64 para mostrar que a torre «não tem tecto» — mas varrer três andares mede três andares; o que não tem tecto é o PASSO, e o passo prova-se por indução."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T19:18:27.509Z
---

Medi a torre de Cayley–Dickson até dim 64, mostrei que em 16, 32 e 64 a norma está
partida e a involução intacta, e escrevi «do lado do dual não tem tecto». O Aarão:
**«a torre nao tem limite dimensional le corpo unversal e peano denovo»**.

**O defeito não era o facto — era o método.** Três andares são três andares. O tecto que
aparecia na minha tabela era o do **meu array** (`TR_MAX`), não o do objeto, e apresentar
um como se fosse o outro é trazer a minha régua como se fosse do mundo.

E o `corpo-estelar.tex` §328 já dizia como se faz: «o passo dos tecidos T_{k+1} = T_k+T_k*
é a estrela usada como **CONSTRUTOR**, e a torre que ele gera não tem topo por dentro:
a indução não pára — a régua é infinita —, e ν∘ν = id fecha cada andar com resíduo 0».

## A forma certa: medir o PASSO, não os andares

    BASE   n = 1: ν é a identidade
    PASSO  ν(a,b) = (ν(a), −b)   e   N(a,b) = N(a) + N(b)
    LOGO   ν∘ν = id em TODO andar — e a conclusão não menciona nenhum array.

É a MESMA correção do `ker T* = (im T)°`: provar a cadeia em vez de varrer os extremos
([[feedback-medir-os-extremos]]). Só que aqui repeti o erro **depois** de o ter
catalogado — e a forma nova foi «subir o limite do array» em vez de «varrer mais casos»,
o que o disfarçou.

## O gatilho

**Quando a tese contém «todo», «sempre» ou «não tem limite», uma varredura NÃO a mede.**
Perguntar: *a minha conclusão menciona um número que veio do meu código?* Se `TR_MAX`,
`lim`, `N_MAX` aparecem na conclusão — ou a determinam —, medi a máquina.

E o corolário: declarar o tecto da máquina **à parte** do da matemática, no mesmo
relatório. Ficam os dois visíveis e ninguém os troca.

## E o que o paper avisa por cima disto

«**Não trazer um morto por régua.**» A leitura clássica diz que a torre para em oito:
verdade **para um ramo, e um só**. Hurwitz *classifica* as álgebras de composição
BILINEARES e **não proíbe coisa nenhuma**; e o √ que aparece ao forçar a norma da esfera
para dentro dos inteiros é «o preço da régua que se trouxe».

Ver [[project-o-fecho-do-dual-lagrange]], [[feedback-o-teto-nao-verificado]],
[[feedback-varrer-onde-nada-pode-falhar]].

**E a razão, na formulação do Aarão:** eu tinha começado a escrever um medidor
para «medir a indecidibilidade», e ele parou-me com uma pergunta:

> «então tu querias validar todo o infinito, é isso? Se isso fosse possível, para que então
>  a matemática? Vamos demitir a matemática e contratar o Claude no lugar.»

**A matemática existe PORQUE o infinito não se enumera.** Se varrer bastasse, não haveria
teorema a provar — bastava esperar. Portanto:

    UM MEDIDOR NUNCA PROVA UM ∀.
    Ele REFUTA (um caso basta, e é conclusivo), ou serve de CONTROLO a uma prova
    que existe noutro sítio.

É a regra que a casa já tinha em `thm:quantificador` — «a indução é conservação: uma
verificação do passo substitui infinitas; a varredura completa é só o controlo» —, e o que
eu fazia era o oposto: varrer e depois escrever o universal. Cada vez que faço isso estou a
fazer exactamente a coisa que tornaria a matemática dispensável.

**E o corolário sobre contraexemplos**, que eu também tinha ao contrário: como refutar é
finito e conclusivo, **UM contraexemplo é uma conclusão completa** — prova a NECESSIDADE da
hipótese. Escrever «daqui não se conclui nada» a seguir a um contraexemplo é atirar fora o
único lado do quantificador que uma máquina pode fechar.
