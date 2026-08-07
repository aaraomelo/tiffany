---
name: project-checkpoint-2026-08-06
description: "Fecho de 06/08 — a assistente ganhou o lado que faltava, e o dia foi de DESFAZER: 15 asserções vazias, um facto invertido, um número 15× errado, e a doutrina usada como aval"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-06T23:59:13.586Z
---

**Fecho de 06/08 — no ar, e o dia não foi de construir: foi de desfazer.**

```
bateria   307 : 307 verdes, 0 falhas       (306 -> 307)
teoria 58 · catalogo 449 · enredo 365 · livro 876 · cv 7 pp.
17 commits · tudo publicado e verificado com content-type
```

## O que se construiu: a assistente ficou com os dois lados

O desenho é dele: **entrada = cone (compacto, desdobra), saída = espiral (o banco)** — involução na entrada, evolução no banco.

- **A involução na entrada**: «vezes» **é** o `x`. A assistente desdobrava `3 x 3` e dizia «não sei» a `3 vezes 3`. O desdobramento **não toca no caminho do corpus** — só o ramo das contas vê a forma desdobrada, senão a fala do corpus deixava de se encontrar a si própria.
- **A evolução no banco — o dual que faltava.** A erosão e a dilatação tratavam **ambas** a fala que tem *a mais*; nenhuma acrescenta. Medido: com uma palavra a mais, **252/252**; com uma a menos, **ZERO**. Agora a fala cabe e o banco desce o resto sozinho: **0 → 216 de 252, e a resposta é a MESMA em 216 de 216**. O número que importa é o **zero diferenças**, não o 216.
- **A bifurcação**: onde ramifica, ela não escolhe — declara os ramos. `conversa.c` 30 → 37 unidades.

## O padrão do dia: asserções que não podiam falhar

- **`aya.c` chegou com 19 unidades VERDES.** Invertida a tese central (E → OU), **QUINZE continuavam verdes**. Quatro secções comparavam literais escritos três linhas acima — a pior: `S12=0,42; S21=0,42; adjuntos=(S12==S21)`, o mesmo número comparado consigo próprio **com forma de medição de reciprocidade**. Reescrito: **19 → 6 unidades**, cada uma com mutação que a mata, **8 doubles → zero**.
- **E recaí dentro da própria correcção**: corrigi §A5–A8 e deixei §A1–A4 a testar que um `AND` faz `AND`; e mantive um `G_aya = 193` sem origem nenhuma a produzir um «cai 40 vezes» que subia ao catálogo como se fosse cálculo. Ver [[feedback-a-referencia-escrita-a-mao]].
- **`smartcontract.c` imprimia a coluna «π(q)» com a variável `passos`** — a mesma da primeira coluna. Insinuava concordância com um número clássico mostrando a mesma coluna duas vezes. Agora Pisano sai da recorrência de Fibonacci mod q: dois caminhos que **podem** discordar.

## O erro factual que corta contra o próprio argumento

O catálogo dizia que Aya *«aumenta a sincronização cortical»* e apoiava nisso o argumento do MEG (que precisa de ~10⁶ neurónios **em fase**). **A literatura diz o contrário**: queda de alfa e aumento de entropia — **dessincronização**.

**Não foi remendado.** Ficou escrito que a correcção **destrói o argumento construído em cima dela**, e que o papel de `G₁` é leitura estrutural e não mecanismo demonstrado.

## A doutrina usada como aval — e a frase que a protegia vinha depois

O acto III tem 20 capítulos com os nomes dos hinos do hinário **Nova Dimensão** (Padrinho Alfredo) — **20/20 na ordem exacta**, conferido contra o PDF.

Mas a construção **«não é metáfora: é»** aparecia **três vezes, todas no acto III** (as outras 15 800 linhas do livro: zero), a converter afirmação doutrinária em facto do projecto: *«o hino que observa o rigor das profecias É a bateria»*, *«Vamos todos renascer não é metáfora: é o que acontece a cada corrida»*. E o acto tinha *«o hinário não prova a bateria»* — **depois** de os capítulos fazerem quatro vezes o contrário. Desfeitas. **Ficou a que não era defeito**: «a ciranda tem um corpo — não metáfora: substância», que afirma o literal e não empresta autoridade.

**E o hinário ficou FORA do repo público** (não estava ignorado, ia subir). O enredo usa-o bem: dá crédito, declara que foi encontrado depois, e **não reproduz um único verso** — medido.

## Minerar os repos desfez um número 15× errado

O enredo dizia **«mil cento e onze ficheiros»** com os nomes que atravessaram as casas. Medido: **76, em quatro das seis**. E os agentes reais do `broca-so` chamam-se **Conversa, Paper, Pesquisa, Software** — Ada/Penny/Alonzo/Caelum são do romance. *(O «1117 registos» do hiper bate exacto.)*

## O deploy, e a suposição que fiz mal

Ver [[project-deploy-sem-github]]: o Actions em `major_outage`, cinco corridas mortas, **o runner local NÃO resolve** (também precisa do serviço de orquestração), e a saída foi `rsync` por SSH com `--exclude repo.git`.

Ver [[feedback-assercoes-vazias]], [[feedback-dual-exige-dois]], [[feedback-insinuacao-arquitetonica]], [[project-assistente-cone-espiral]].
