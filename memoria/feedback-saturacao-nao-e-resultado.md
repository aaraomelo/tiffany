---
name: feedback-saturacao-nao-e-resultado
description: "«Saturação não é resultado, é falha vestida de teorema» — e o número grande é sintoma de prova por CRESCIMENTO."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-16T00:11:51.267Z
---

Iterei Newton oito vezes a partir de 2, cinco sobreviveram, e ia publicar **«5 de 8»**
como resultado. O Aarão: *«não tem resultado como saturação, isso não é resultado, é
falha travestida de teorema»*.

As três que faltavam não falharam por matemática: Newton **duplica os dígitos** a cada
passo, e ao sexto o numerador não cabia num `long`. O que eu tinha medido era o tamanho
do tipo.

**Why:** dar estatuto de resultado ao transbordo é o pior tipo de asserção vazia — passa
verde, parece conteúdo, e é uma afirmação sobre a minha aritmética disfarçada de
afirmação sobre o objecto.

**How to apply:**
1. **A tese é do PASSO, não da órbita.** A regra já era da casa: *tese com «sempre» não
   se varre, prova-se o passo*. Trocar «iterar 8× a partir de 2» por «todo majorante tem
   um menor», verificado em testemunhas independentes: passou de 5/8 para **1040/1040 e
   560/560**, e nenhum número foi obrigado a crescer. E prova MAIS: vale para qualquer b.
2. **Contar a saturação num sítio SEPARADO dos defeitos.** Se ficar na mesma coluna, o
   `long` a transbordar aparece como a matemática a falhar.
3. **Segunda realização independente** quando a primeira atinge o limite. Medido: a
   exacta satura em k=19; a modular responde até k=400 em três primos.
4. **Um número grande é sintoma de prova por crescimento.** Foi assim que apareceu o
   `fn_bissec` a correr 40 passos com o denominador a duplicar: os últimos nove corriam
   sobre inteiros já enrolados e a função devolvia 1 na mesma — uma saturação a sair como
   veredicto («o c é irracional»). Agora tem tecto e conta em `cl_saturou`.

Instrumento permanente: `-DQZ_MEDE` no `racionais.h` regista a maior magnitude e o maior
produto cruzado que passaram — a pergunta «cabe no tipo?» feita ao próprio código.
Medido: a assistente chegava a **8,4×10¹⁸**, 91% do tecto do `long`; e só **4 de 362**
medidores usam `Qz`. Ver [[project-teorema-do-gato]], [[feedback-o-numero-que-nao-cabe]],
[[feedback-o-tecto-do-array]].
