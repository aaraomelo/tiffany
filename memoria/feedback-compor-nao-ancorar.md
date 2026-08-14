---
name: compor-nao-ancorar
description: No tradutor NADA se posiciona à mão — todo anexo compõe pelo motor da espiral com a SEMENTE certa; âncoras/offsets meus são invenção e ele apanha-as na hora
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 0d72a882-4e0b-4eb2-bdba-8a2376988e06
  modified: 2026-08-11T19:10:33.038Z
---

Nos limites do integral esticado (11/08) escrevi âncoras à mão («a caixa do sup encosta
o topo à ponta») com offsets `topo − asc`, empilhei no mesmo x, avancei pelo máximo — e
quebrou em `∫_{f(a)}^{f(b)}` (limites multi-corrida colapsaram uns sobre os outros,
teoria.tex:1393). Ele: «vc não está seguindo a composição, colocou fixo tudo, um monte
de aproximação; aplica a teoria; não se inventa nada; estou falando a mesma coisa há 1 hr».

**Why:** o sistema tem UM motor de composição — a espiral (`esp_gira`/`esp_escala`/
`esp_sobe`, o vector Re=kern/Im=passo, conservação por construção: a translação recebe o
que a escala tirou). Um anexo (limite, expoente, rótulo) nunca se coloca: **compõe-se pelo
caminho comum, mudando só a SEMENTE do giro**. A fronteira composta (∫ pelo vão, k medido)
entra como semente `corpo·k` — um produto cruzado — e escala E translação saem juntas do
mesmo giro. Em k=1 a lei devolve o expoente comum sozinha, sem caso especial.

**How to apply:** antes de posicionar qualquer coisa no tex_core, perguntar: «qual é a
semente deste giro?» — nunca «onde encosto esta caixa?». Se aparece um offset meu
(`− asc/2`, `+ dv/3` inventado, «empilha no mesmo x»), é o sintoma. O mesmo vale para o
tamanho: fronteira compõe-se DA ASSINATURA à medida da região (não há glifo de corpo
fixo; o relógio avalia a curva no grau que a região pede — π sai da dimensão, nada se
calcula). Ver [[feedback-a-base-ja-existe]] e [[feedback-a-ausencia-e-deliberada]] — é a
mesma raiz: trazer régua de fora para um objeto que já tem a sua.

**E a segunda volta do mesmo erro (mesma hora):** meti o k da fronteira na SEMENTE
inteira do giro — os limites saíram enormes («os limites das integrais enormes, vc não
está ajustando a escala»). A lei é a dos DUAIS, e a transformada dourada (catalogo
sec:dourada) dá a frase: **«dilatar vira transladar»** — a dilatação (multiplicativa) da
fronteira passa ao lado dual como TRANSLAÇÃO (aditiva), um produto cruzado no inteiro; a
escala do glifo fica no seu degrau da espiral. Não se compõe exponencial nenhuma: o
relógio já o faz por involução. Quando um fator aparecer, perguntar DE QUE LADO do par
aditivo/multiplicativo ele age — pôr no lado errado é o defeito.

**E o vocabulário importa (terceira volta):** chamei «vector de respiro» ao desvio dos
limites e ele corrigiu: «não tem vetor de respiro — tem uma SEMENTE DE ESPAÇAMENTO
inicial que se propaga sozinha pelas involuções de escala». O espaçamento nunca se põe
num sítio: a semente declara-o UMA vez e `sem_resp(passo do degrau)` é a semente LIDA
naquele degrau — a involução de escala dá o passo, a semente dá a quota. Se estou a
«adicionar um termo» em vez de «ler a semente no degrau», o enquadramento está errado
mesmo que o número seja igual.
