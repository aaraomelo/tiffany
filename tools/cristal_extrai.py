#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""cristal_extrai.py — arqueologia do cristal grande (eval.txt, 13/08).

O cristal do projeto vive no broca-so: conversa/dados/conhecimento.graph.jsonl,
um jornal de 75.165 registos onde cada conceito foi reescrito ao longo do tempo.
A extração toma a ÚLTIMA versão de cada id (4286 conceitos), em ordem canónica,
e escreve cristal/cristal.jsonl no tiffany — a fonte da recuperação.

Regra da casa (tools/corpo.sh): «IP privado — não publicar». O user@IP do
servidor é substituído por «gex44» e a substituição é contada à vista.

O jornal original NÃO se toca: fica no broca-so, com a história inteira.

    python3 tools/cristal_extrai.py
"""
import json, os, sys

FONTE = '/home/aaraolopes/Documentos/broca-so/conversa/dados/conhecimento.graph.jsonl'
DEST = os.path.join(os.path.dirname(__file__), '..', 'cristal', 'cristal.jsonl')

# o que não se publica (tools/corpo.sh: «IP privado — não publicar»)
PRIVADO = [('aarao@78.46.19.151', 'gex44'), ('78.46.19.151', 'gex44')]


def canonico(r):
    return json.dumps(r, ensure_ascii=False, sort_keys=True, separators=(',', ':'))


def main():
    last, versoes, total = {}, {}, 0
    with open(FONTE, encoding='utf-8') as f:
        for linha in f:
            total += 1
            r = json.loads(linha)
            last[r['id']] = r
            versoes[r['id']] = versoes.get(r['id'], 0) + 1
    limpos = 0
    saida = []
    for i in sorted(last):
        s = canonico(last[i])
        for privado, publico in PRIVADO:
            if privado in s:
                s = s.replace(privado, publico)
                limpos += 1
        saida.append(s)
    os.makedirs(os.path.dirname(DEST), exist_ok=True)
    with open(DEST, 'w', encoding='utf-8') as f:
        f.write('\n'.join(saida) + '\n')
    # a história não desaparece na normalização (o floxina_investigacao tem
    # 2908 versões no jornal): a versão canónica sabe que tem passado.
    HIST = os.path.join(os.path.dirname(DEST), 'historia.tsv')
    with open(HIST, 'w', encoding='utf-8') as f:
        f.write('# id\tversoes_no_jornal (broca-so/conversa/dados/conhecimento.graph.jsonl)\n')
        for i in sorted(versoes):
            f.write(f'{i}\t{versoes[i]}\n')
    print(f'jornal: {total} registos; conceitos: {len(saida)}; privado limpo: {limpos}')
    print(f'-> {os.path.normpath(DEST)}')
    print(f'-> {os.path.normpath(HIST)} (soma das versões: {sum(versoes.values())})')
    return 0


if __name__ == '__main__':
    sys.exit(main())
