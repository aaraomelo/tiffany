---
name: feedback-o-limiar-que-o-texto-despediu
description: "DOZE asserções em que o texto declara que o limiar saiu — «o 1e-15 dava folga», «não precisa de régua» — e ele continua na condição, ao lado da medida exacta."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-17T15:38:42.202Z
---

Doze asserções, num dia, com a mesma forma:

```c
ok("... e o 1e-14 dava folga a uma identidade que sai do termo constante",
   fabs(s1*sl1 + 1.0) < 1e-14 && pza == -4 && pzb == 0);   // o 1e-14 AINDA LÁ ESTÁ
```

O texto **declara que o limiar saiu**; a condição **mantém-no**, agora somado à medida
exacta que o substituiu. As frases eram estas, e todas minhas:

| ficheiro | o texto diz | a condição tinha |
|---|---|---|
| `furos.c` | «mede-se **sem logaritmos**» | `fabs(ultima − log2/log3) < 1e-9` |
| `octeto.c` | «dois acos a mascarar x = x» | `fabs(a3 − previsto3) < 1e-9` |
| `plugs.c` | «sem limiar. O 1e-14 dava folga» | `fabs(s1*sl1 + 1) < 1e-14` |
| `telomero.c` | «sem o 1e-9, que dava folga» | `fabs(v − 137.0/4.0) < 1e-9` |
| `escada.c` | «o que se mede **não** é \|0\| < 1e-15» | `fabs(n_inv) < 1e-15` |
| `lambert.c` | «comparar com 1e-15 media **zero vezes**» | `cabs(dcrit) < 1e-15` |
| `simula.c` | «era do **arredondamento**, não do facto» | `nrm(soma) < 1e-15` |
| `tesserato_espiral.c` | «o 1e-12 **tolerava** o arredondamento» | `fabs(ang − 2π) < 1e-12` |
| `dif.c` | «nenhuma **precisa de régua**» | `fabs(...) < 1e-12` |
| `solar.c` | «o 1e-15 dava folga a uma igualdade que não tem» | `fabs(THD2 − 1/φ) < 1e-12` |
| `travessia.c` | «dava folga ao arredondamento dos N logaritmos» | `fabs(hH − h0) < 1e-9` |
| `spline.c` | «sem o 1e-18, que dava folga a um zero» | `eu < 1e-18` |

**Why:** a correcção **acrescenta** a medida exacta e **não tira** a que ela substitui. E
depois é invisível, por três razões que se somam:

1. **O comentário parece resolvido.** Quem relê — eu — vê a frase a explicar o defeito e
   dá o assunto por fechado. O texto é uma confissão, e eu li-a como um recibo.
2. **A asserção continua verde.** Somar `A_exacto && B_folgado` não muda o resultado
   enquanto os dois valerem; o limiar sobrevive por não incomodar.
3. **O gume não o apanha** se for apontado ao bloco: mutar o dado derruba pela parte
   exacta, e a parte folgada nunca é testada. Ver [[feedback-o-gume-por-lei]].

**How to apply:**
1. **Ao corrigir um limiar, apagá-lo.** Não basta acrescentar a medida inteira ao lado —
   a condição fica com um termo que não pode falhar somado a um que pode.
2. **O detector é barato e apanha os doze:** o texto do `ok(...)` fala de «sem limiar»,
   «sem raiz», «dava folga», «tolerava», «não precisa de régua», e a CONDIÇÃO contém
   `fabs`/`sqrt`/`log`/`1e-`. Correr sempre depois de qualquer ronda de correcções.
3. E a variante pior, no `furos.c`: além do limiar, `ultima = log(2^k)/(k·log3)` compara-se
   com `log(2.0)/log(3.0)` — os `k` **cancelam-se**, é x == x. De seis termos, quatro não
   podiam falhar. Ver [[feedback-assercoes-vazias]].

Ver [[feedback-a-correccao-onde-nasceu]], [[feedback-o-limiar-tem-tres-causas]],
[[feedback-representacao-inteligente]].


---

## 18/08/2026 — a correcção que DESCREVE o defeito e o reintroduz na linha seguinte

O caso mais puro que apanhei disto, em `simula.c`. A nota que lá estava, escrita por mim numa
sessão anterior, dizia:

> *(O que aqui se media era `janela_aberta·ganho > 100·janela_aberta`, isto é `ganho > 100`: a
> comparação não via a janela.)*

Diagnóstico certo. E a condição «corrigida», logo por baixo:

```c
ganho_malha > 100.0 && janela_fechada > janela_aberta
```

com `janela_fechada` **definida duas linhas acima** como `janela_aberta * ganho_malha`. A
segunda metade é `ganho > 1` — a mesma tautologia, agora com uma nota por cima a explicar
porque é que ela é uma tautologia.

**O que falhou não foi o diagnóstico: foi o gume.** Eu escrevi a nota, escrevi a condição nova,
e não perguntei *que entrada faria isto falhar*. A nota deu-me a sensação de ter resolvido.

### A regra

Depois de corrigir uma asserção vazia, **a correcção leva gume, como se fosse nova**. A nota
explicativa não substitui a mutação — e quanto mais convincente a nota, mais preciso é o
gume, porque ela é o que me impede de reler a condição com olhos frios.

E quando a lei é do tipo «X alarga com Y», a forma que morde quase nunca é uma desigualdade:
é uma **conservação**. Aqui era «a janela cresce por G, a banda encolhe por G, e o produto não
se move» — três lados em ℤ, três gumes que mordem, e nenhum deles mordia na versão anterior.

Relacionado: [[feedback-assercoes-vazias]] (a nona forma é a irmã desta),
[[feedback-o-gume-por-lei]].
