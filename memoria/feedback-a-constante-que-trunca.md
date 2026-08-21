---
name: feedback-a-constante-que-trunca
description: "Trocar o TIPO e deixar a constante em vírgula: o literal trunca para zero e a asserção passa a comparar contra zero — tautologia, sempre verde."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: cee0686a-c852-4d69-8fca-ca206e1fba24
  modified: 2026-08-20T23:08:54.346Z
---

A Fase A trocou `double` por `long` em todo o repo e deixou os LITERAIS. Cinco
asserções viraram tautologias, e nenhuma podia cair:

    long f = 0.1 + 0.2;   int falha = (f != 0.3);     → f = 0, compara «0 ≠ 0.3»
    long med = 1.0; med = med*2.0/3.0;                → 0 no 1.º nível de Cantor
    long t = 0.7;  … s*t/p …                          → exp(tA) fica a IDENTIDADE
    const long kT = 1.38e-23*300*0.693;               → 0, e os DOIS lados de
                                                        Landauer imprimem 0
    long rtt = 0.5;                                   → 0, a tabela toda em double
                                                        impressa com %ld

**Why:** o literal não é decoração da constante — é a ARITMÉTICA. O compilador
promove tudo a double, faz a conta em vírgula, e trunca só na atribuição. O valor
que sobra é zero, e zero passa em qualquer comparação escrita para «é pequeno».
O texto impresso continua a dizer «t = 0,7» ao lado do zero.

**How to apply:** ao migrar um tipo, migrar as CONSTANTES na mesma passagem — e a
verificação é gratuita: `grep` por literais de vírgula fora de comentários e
strings. Quando o valor é físico, a saída é a UNIDADE, não a vírgula: kT·ln2 = 0 J
vira 2 870 978 rJ (10⁻²⁷ J), exacto, com k_B exacto pelo SI e ln2 declarado como o
único factor arredondado ([[feedback-genealogia-das-constantes]]).

E quando o objecto pede vírgula de verdade, a resposta é a da casa e é melhor: a
falha do flutuante mede-se nos BITS (`le_f64_bits_pq`, inteiro), e exp(tA)
truncada — que não é ortogonal nem em ℝ — dá lugar a CAYLEY, exacto em ℤ.

O sintoma: uma asserção sobre um valor pequeno que passa, e o número impresso ao
lado é 0. Ver [[feedback-o-tipo-nao-e-a-aritmetica]] e
[[feedback-assercoes-vazias]].
